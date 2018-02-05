#include "libvpk.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    VPKHandle handle = vpk_load("D:/Steam/steamapps/common/Portal 2/portal2/pak01");

	VPKFile file = vpk_fopen(handle, "sound/alarms/klaxon1.wav");

	char* fileData = vpk_malloc_and_read(file);

	FILE* extractedFile = fopen("klaxon1.wav", "wb+");
	fwrite(fileData, vpk_flen(file), 1, extractedFile);
	fclose(extractedFile);

	vpk_free(fileData);

    vpk_close(handle);

    return 0;
}