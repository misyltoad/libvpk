cmake_minimum_required(VERSION 3.9)

add_library (libvpk SHARED src/libvpk.c include/libvpk.h)
target_include_directories(libvpk PUBLIC include extern/Containment)
target_compile_definitions(libvpk PRIVATE LIBVPK_EXPORTS _CRT_SECURE_NO_WARNINGS)

add_library (libvpk_static STATIC src/libvpk.c include/libvpk.h)
target_include_directories(libvpk_static PUBLIC include extern/Containment)
target_compile_definitions(libvpk_static PRIVATE _CRT_SECURE_NO_WARNINGS)

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "-static -static-libgcc -static-libstdc++")
endif(UNIX)

SET_TARGET_PROPERTIES(libvpk PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(libvpk_static PROPERTIES PREFIX "")