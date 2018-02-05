#include "libvpk.h"
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#include <Windows.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

int main(int argc, char** argv)
{
    VPKHandle handle = vpk_load("D:/Steam/steamapps/common/Portal 2/portal2/pak01");
	//VPKHandle handle = vpk_load("D:/Steam/steamapps/common/Portal 2/portal2/pak01_dir.vpk");

	VPKFile file = vpk_fopen(handle, "models/ballbot_animations.ani");

	VPKFile loopFile = vpk_ffirst(handle);

	char* fileData = vpk_malloc_and_read(file);

	FILE* extractedFile = fopen("klaxon1.wav", "wb+");
	fwrite(fileData, vpk_flen(file), 1, extractedFile);
	fclose(extractedFile);

	vpk_free(fileData);

    vpk_close(handle);

    return 0;
}