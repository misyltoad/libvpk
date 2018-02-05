cmake_minimum_required(VERSION 3.10)

add_library (libvpk SHARED src/libvpk.c include/libvpk.h)
target_include_directories(libvpk PUBLIC include extern/Containment)
target_compile_definitions(libvpk PRIVATE LIBVPK_EXPORTS _CRT_SECURE_NO_WARNINGS)

add_library (libvpk_static STATIC src/libvpk.c include/libvpk.h)
target_include_directories(libvpk_static PUBLIC include extern/Containment)
target_compile_definitions(libvpk_static PRIVATE _CRT_SECURE_NO_WARNINGS)

add_executable(libvpk_test test/main.c)
target_include_directories(libvpk PUBLIC include)
target_link_libraries(libvpk_test PRIVATE libvpk)