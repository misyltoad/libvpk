#include "libvpk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <Containment.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct
{
	struct VPKHandleData* Handle;
	char* FullFormedPath;
	char* Extension;
	char* Folder;
	char* FileName;
	VPKFileMeta Meta;
	char* Preload;
	FILE* File;
} VPKFileData;

DEFINE_CONTAINMENT_TYPES(VPKFileData);

typedef struct
{
	char* BasePath;
	FILE* DirectoryFile;
	VPKGenericHeader* Header;
	VPKFileData_Hashmap FileMap;
} VPKHandleData;

int vpk_valid_header(VPKGenericHeader* header)
{
	if (!header || header->Signature != VPK_SIGNATURE)
		return 0;

	return 1;
}

int vpk_valid_handle(VPKHandle handle)
{
	return handle == VPK_NULL_HANDLE ? 0 : 1;
}

const char* vpk_fext(VPKFile file)
{
	VPKFileData* handle_data = (VPKFileData*)file;
	return handle_data->Extension;
}

const char* vpk_fpath(VPKFile file)
{
	VPKFileData* handle_data = (VPKFileData*)file;
	return handle_data->FullFormedPath;
}

static inline VPKHandleData* vpk_data_from_handle(VPKHandle handle)
{
	return (VPKHandleData*)handle;
}

char* vpk_read_string(FILE* file)
{
	char string_buffer[256];

	int ch, i = 0;
	while ((ch = fgetc(file)) != '\0')
		string_buffer[i++] = ch;

	string_buffer[i++] = '\0';

	char* string = (char*)malloc(i);
	memcpy(string, string_buffer, i);

	return string;
}

size_t_Hashmap vpkIndexes;

VPKFile vpk_ffirst(VPKHandle handle)
{
	VPKHandleData* handle_data = vpk_data_from_handle(handle);
	size_t_Hashmap_Set(&vpkIndexes, (uint64_t) handle, 0);
	return (VPKFile)&handle_data->FileMap.data.data[0].value;
}

VPKFile vpk_fnext(VPKHandle handle)
{
	VPKHandleData* handle_data = vpk_data_from_handle(handle);

	size_t index = size_t_Hashmap_Get(&vpkIndexes, (uint64_t) handle, 0);
	index++;

	size_t_Hashmap_Set(&vpkIndexes, (uint64_t) handle, index);

	if (index < handle_data->FileMap.data.length)
		return (VPKFile)&handle_data->FileMap.data.data[index].value;
	else
		return NULL;
}

static int32_t vpk_read_int32(FILE* file)
{
	int32_t retVal;
	fread(&retVal, sizeof(int32_t), 1, file);
	return retVal;
}

static int16_t vpk_read_int16(FILE* file)
{
	int16_t retVal;
	fread(&retVal, sizeof(int16_t), 1, file);
	return retVal;
}

static void vpk_parse_directories(VPKHandleData* data)
{
	data->FileMap = VPKFileData_Hashmap_Create();

	for (;;)
	{
		char* extension = vpk_read_string(data->DirectoryFile);

		if (extension[0] == '\0')
			break;

		for (;;)
		{
			char* path = vpk_read_string(data->DirectoryFile);

			if (path[0] == '\0')
				break;

			for (;;)
			{
				char* file_name = vpk_read_string(data->DirectoryFile);
				if (file_name[0] == '\0')
					break;

				VPKFileData vpk_file;
				vpk_file.File = NULL;
				vpk_file.Handle = (VPKHandle)data;
				vpk_file.Extension = extension;
				vpk_file.FileName = file_name;
				vpk_file.Folder = path;

				size_t size = snprintf(NULL, 0, "%s/%s.%s", path, file_name, extension);
				size++;
				vpk_file.FullFormedPath = (char*)malloc(size);
				snprintf(vpk_file.FullFormedPath, size, "%s/%s.%s", path, file_name, extension);

				vpk_file.Meta.CRC = vpk_read_int32(data->DirectoryFile);
				vpk_file.Meta.PreloadBytes = vpk_read_int16(data->DirectoryFile);
				vpk_file.Meta.ArchiveIndex = vpk_read_int16(data->DirectoryFile);

				//if (vpk_file.Meta.ArchiveIndex == 0xffff)

				vpk_file.Meta.Offset = vpk_read_int32(data->DirectoryFile);
				vpk_file.Meta.Length = vpk_read_int32(data->DirectoryFile);
				int16_t terminator = vpk_read_int16(data->DirectoryFile);
				vpk_file.Preload = NULL;
				if (vpk_file.Meta.PreloadBytes)
				{
					vpk_file.Preload = (char*)malloc(vpk_file.Meta.PreloadBytes);
					fread(vpk_file.Preload, 1, vpk_file.Meta.PreloadBytes, data->DirectoryFile);
				}

				uint64_t hash = hash_str(vpk_file.FullFormedPath);
				VPKFileData_Hashmap_Set(&data->FileMap, hash, vpk_file);
			}
		}
	}
}

static int strcicmp(char const *a, char const *b)
{
	for (;; a++, b++) {
		int d = tolower(*a) - tolower(*b);
		if (d != 0 || !*a)
			return d;
	}
}

VPKHandle vpk_load(const char* vpk_path)
{
	size_t strlen_vpk_path = strlen(vpk_path);
	const char* dot_vpk = &vpk_path[strlen_vpk_path - 4];
	char underscore = vpk_path[strlen_vpk_path - 8];

	char shouldSortOutEnding = !strcicmp(dot_vpk, ".vpk") && underscore == '_';

	char* full_path;
	if (shouldSortOutEnding)
	{ 
		size_t size = strlen_vpk_path + 1;
		full_path = malloc(size);
		memcpy(full_path, vpk_path, size);
		full_path[strlen_vpk_path - 8] = '\0';
		strcat(full_path, "_dir.vpk");
	}
	else
	{
		size_t size = snprintf(NULL, 0, "%s_dir.vpk", vpk_path);
		size++;
		full_path = malloc(size);
		snprintf(full_path, size, "%s_dir.vpk", vpk_path);
	}

	VPKHandleData* data = (VPKHandleData*)malloc(sizeof(VPKHandleData));

	data->DirectoryFile = fopen(full_path, "rb");

	if (!data->DirectoryFile)
	{
		free(full_path);
		return VPK_NULL_HANDLE;
	}

	size_t basePathLen;

	if (shouldSortOutEnding)
	{
		full_path[strlen_vpk_path - 8] = '\0';
		basePathLen = strlen(full_path) + 1;
	}
	else
		basePathLen = strlen_vpk_path + 1;

	data->BasePath = malloc(basePathLen);

	if (shouldSortOutEnding)
		memcpy(data->BasePath, full_path, basePathLen);
	else
		memcpy(data->BasePath, vpk_path, basePathLen);

	free(full_path);

	VPKGenericHeader generic_header;
	fread(&generic_header, sizeof(VPK1Header), 1, data->DirectoryFile);

	rewind(data->DirectoryFile);

	size_t header_size = generic_header.Version == 2 ? sizeof(VPK2Header) : generic_header.Version == 1 ? sizeof(VPK1Header) : 16;
	data->Header = (VPKGenericHeader*)malloc(header_size);

	fread(data->Header, header_size, 1, data->DirectoryFile);

	if (vpk_valid_header(data->Header) == 0)
	{
		vpk_close((VPKHandle)data);
		return VPK_NULL_HANDLE;
	}

	vpk_parse_directories(data);

	return (VPKHandle)data;
}

VPKFileData* vpk_get_file_data(VPKHandle handle, const char* name)
{
	VPKHandleData* handle_data = vpk_data_from_handle(handle);

	size_t length = strlen(name) + 1;
	char* name_with_fixed_slash = (char*)malloc(length);
	memcpy(name_with_fixed_slash, name, length);

	for (size_t i = 0; i < length - 1; i++)
		if (name_with_fixed_slash[i] == '\\')
			name_with_fixed_slash[i] = '/';

	VPKFileData* file_data = VPKFileData_Hashmap_GetPtr(&handle_data->FileMap, hash_str(name_with_fixed_slash), NULL);
	free(name_with_fixed_slash);
	return file_data;
}

VPKFile vpk_fopen(VPKHandle handle, const char* name)
{
	VPKHandleData* handle_data = vpk_data_from_handle(handle);
	VPKFileData* file_data = vpk_get_file_data(handle, name);

	size_t size = snprintf(NULL, 0, "%s_%03d.vpk", handle_data->BasePath, file_data->Meta.ArchiveIndex);
	size++;
	char* full_path = malloc(size);
	snprintf(full_path, size, "%s_%03d.vpk", handle_data->BasePath, file_data->Meta.ArchiveIndex);

	file_data->File = fopen(full_path, "rb");

	free(full_path);

	fseek(file_data->File, file_data->Meta.Offset, SEEK_SET);

	return file_data;
}

void vpk_fmeta(VPKFileMeta* out, VPKFile file)
{
	VPKFileData* file_data = (VPKFileData*)file;
	memcpy(out, &file_data->Meta, sizeof(VPKFileMeta));
}

size_t vpk_flen(VPKFile file)
{
	VPKFileData* file_data = (VPKFileData*)file;
	return file_data->Meta.Length + file_data->Meta.PreloadBytes;
}

size_t vpk_fread(void* pointer, size_t size, size_t count, VPKFile file)
{
	char* ptr = (char*)pointer;
	VPKFileData* file_data = (VPKFileData*)file;

	long total_length = (long) (size * count);

	long cursor = ftell(file_data->File) - file_data->Meta.Offset;

	if (cursor + total_length > file_data->Meta.Length + file_data->Meta.PreloadBytes)
		return 0;

	long preload_cursor = MIN(file_data->Meta.PreloadBytes, cursor);
	long chunked_cursor = MAX(cursor - file_data->Meta.PreloadBytes, 0);

	fseek(file_data->File, file_data->Meta.Offset + (long) chunked_cursor, SEEK_SET);

	long preload_read_count = MIN(preload_cursor + total_length, file_data->Meta.PreloadBytes) - preload_cursor;
	long chunked_read_count = MIN(chunked_cursor + total_length, file_data->Meta.Length) - chunked_cursor;

	if (preload_read_count > 0)
	{
		memcpy(ptr, file_data->Preload, preload_read_count);
		ptr += preload_read_count;
	}

	if (chunked_read_count > 0)
		fread(ptr, size, chunked_read_count, file_data->File);

	return 1;
}

void vpk_fclose(VPKFile file)
{
	VPKFileData* file_data = (VPKFileData*)file;

	if (file_data->File)
		fclose(file_data->File);

	file_data->File = NULL;
}

char* vpk_malloc_and_read(VPKFile file)
{
	size_t len = vpk_flen(file);
	char* data = (char*)malloc(len + 1);
	size_t read_count = vpk_fread(data, 1, len, file);
	data[len] = '\0';

	return data;
}

void vpk_free(char* data)
{
	free(data);
}

void vpk_close(VPKHandle handle)
{
	VPKHandleData* data = vpk_data_from_handle(handle);

	if (data)
	{
		if (data->FileMap.data.data)
		{
			for (size_t i = 0; i < data->FileMap.data.length; i++)
			{
				static char* lastExtension = NULL;
				static char* lastFolder = NULL;

				if (lastExtension != data->FileMap.data.data[i].value.Extension)
					free(data->FileMap.data.data[i].value.Extension);
				lastExtension = data->FileMap.data.data[i].value.Extension;

				if (data->FileMap.data.data[i].value.File)
					fclose(data->FileMap.data.data[i].value.File);

				if (lastFolder != data->FileMap.data.data[i].value.Folder)
					free(data->FileMap.data.data[i].value.Folder);
				lastFolder = data->FileMap.data.data[i].value.Folder;

				free(data->FileMap.data.data[i].value.FileName);
				free(data->FileMap.data.data[i].value.FullFormedPath);


				if (data->FileMap.data.data[i].value.Preload)
					free(data->FileMap.data.data[i].value.Preload);
			}

			free(data->FileMap.data.data);
		}

		if (data->BasePath)
			free(data->BasePath);

		if (data->FileMap.hashes.data)
			free(data->FileMap.hashes.data);

		if (data->DirectoryFile)
			fclose(data->DirectoryFile);

		if (data->Header)
			free(data->Header);

		free(data);
	}
}

VPKGenericHeader* vpk_get_header(VPKHandle handle)
{
	VPKHandleData* data = vpk_data_from_handle(handle);

	if (data)
		return data->Header;

	return NULL;
}