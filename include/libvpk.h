#ifndef LIBVPK_H
#define LIBVPK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(LIBVPK_EXPORTS) && defined(_WIN32)
#define LIBVPK_EXPORT __declspec(dllexport)
#else
#define LIBVPK_EXPORT
#endif

#define VPK_SIGNATURE 0x55aa1234

	typedef void* VPKHandle;
	typedef void* VPKFile;

	typedef struct
	{
		int32_t Signature;
		int32_t Version;
		int32_t TreeSize;
		int32_t FileDataSectionSize;
		int32_t ArchiveMD5SectionSize;
		int32_t OtherMD5SectionSize;
		int32_t SignatureSectionSize;
	} VPK2Header;

	typedef struct
	{
		int32_t Signature;
		int32_t Version;
		int32_t TreeSize;
	} VPK1Header;

	typedef struct
	{
		int32_t CRC;
		int16_t PreloadBytes;
		int16_t ArchiveIndex;
		int32_t Offset;
		int32_t Length;
	} VPKFileMeta;

	typedef VPK1Header VPKGenericHeader;

#define VPK_NULL_HANDLE ((VPKHandle)0)

	LIBVPK_EXPORT VPKHandle vpk_load(const char* vpk_path);
	LIBVPK_EXPORT void vpk_close(VPKHandle handle);
	LIBVPK_EXPORT VPKGenericHeader* vpk_get_header(VPKHandle handle);
	LIBVPK_EXPORT int vpk_valid_handle(VPKHandle handle);

	LIBVPK_EXPORT const char* vpk_fext(VPKFile file);
	LIBVPK_EXPORT const char* vpk_fpath(VPKFile file);
	LIBVPK_EXPORT VPKFile vpk_ffirst(VPKHandle handle);
	LIBVPK_EXPORT VPKFile vpk_fnext(VPKHandle handle);

	LIBVPK_EXPORT VPKFile vpk_fopen(VPKHandle handle, const char* file);
	LIBVPK_EXPORT void vpk_fmeta(VPKFileMeta* out, VPKFile file);
	LIBVPK_EXPORT size_t vpk_flen(VPKFile file);
	LIBVPK_EXPORT size_t vpk_fread(void* ptr, size_t size, size_t count, VPKFile file);
	LIBVPK_EXPORT void vpk_fclose(VPKFile file);
	LIBVPK_EXPORT char* vpk_malloc_and_read(VPKFile file);
	LIBVPK_EXPORT void vpk_free(char* data);

#ifdef __cplusplus
}
#endif

#endif