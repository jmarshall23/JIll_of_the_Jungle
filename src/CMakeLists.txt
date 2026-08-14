cmake_minimum_required(VERSION 3.21)

project(jill1_recovered VERSION 0.1 LANGUAGES C)

# Jill was a 16-bit DOS program.  This recovery keeps the original 8/16/32-bit
# data widths, but intentionally targets a flat 32-bit address space.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "Jill recovery must be configured for a 32-bit target. "
        "With Visual Studio use: cmake -S src -B build -A Win32")
endif()

set(JILL_SOURCES
    CONFIG.C
    COPYFILE.C
    DESIGN.C
    GAMECTRL.C
    GR.C
    GRASM.C
    HOSTWIN.C
    JINFO.C
    JMAN.C
    JOBJ.C
    JOBJ2.C
    JOBJ3.C
    JPLAYER.C
    JUNGLE.C
    JVOL.C
    KEYBOARD.C
    KEYINTR.C
    MUSIC.C
    MUSINTR.C
    PIXWRITE.C
    PLATFORM.C
    SHM.C
    UNCRUNCH.C
    UNKNOWN.C
    WIN.C
    GAME/GRL.C
)

# Uppercase .C is the recovered DOS filename spelling.  Some toolchains infer
# C++ from that suffix, so force these files to the C frontend.
set_source_files_properties(${JILL_SOURCES} PROPERTIES LANGUAGE C)

add_executable(jill1 ${JILL_SOURCES})
target_compile_features(jill1 PRIVATE c_std_11)

# Keep the deliverable beside the original Jill data files, regardless of
# whether a single- or multi-configuration CMake generator is used.
set_target_properties(jill1 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/.."
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/.."
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/.."
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/.."
    VS_GLOBAL_VcpkgEnabled "false"
    VS_GLOBAL_VcpkgApplocalDeps "false"
)

if(MSVC)
    target_compile_options(jill1 PRIVATE /TC /W4 /wd4100 /wd4204)
    target_compile_definitions(jill1 PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(jill1 PRIVATE user32 gdi32)
else()
    target_compile_options(jill1 PRIVATE -m32 -Wall -Wextra -Wpedantic)
    target_link_options(jill1 PRIVATE -m32)
endif()
