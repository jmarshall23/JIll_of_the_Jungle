cmake_minimum_required(VERSION 3.21)

project(jill1_recovered VERSION 0.1 LANGUAGES C CXX)

# Jill was a 16-bit DOS program.  This recovery keeps the original 8/16/32-bit
# data widths, but intentionally targets a flat 32-bit address space.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "Jill recovery must be configured for a 32-bit target. "
        "With Visual Studio use: cmake -S src -B build -A Win32")
endif()

# SDL2 owns the native window, renderer, keyboard, timer, and host threads.
# Build only its static library and SDL2main so the game remains a single EXE.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/SDL2 EXCLUDE_FROM_ALL)

# The audio boundary uses a vendored, statically linked OpenAL Soft.
# Only WASAPI playback is required by Jill; tools, examples, installation
# rules, capture-only legacy backends, and loadable OpenAL modules are omitted.
set(LIBTYPE STATIC CACHE STRING "Build OpenAL Soft statically" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_HRTF_DATA OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "" FORCE)
set(ALSOFT_EAX OFF CACHE BOOL "" FORCE)
set(ALSOFT_BACKEND_WASAPI ON CACHE BOOL "" FORCE)
set(ALSOFT_BACKEND_DSOUND OFF CACHE BOOL "" FORCE)
set(ALSOFT_BACKEND_WINMM OFF CACHE BOOL "" FORCE)
set(ALSOFT_BACKEND_WAVE OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/openal-soft EXCLUDE_FROM_ALL)

# Jill's CMF files contain AdLib operator patches, not General MIDI program
# numbers.  Build only the vendored LGPL Nuked OPL3 core and run it in its
# OPL2-compatible mode from the recovered WORX register behavior.
add_library(nuked_opl3 STATIC thirdparty/nuked-opl3/opl3.c)
target_include_directories(nuked_opl3 PUBLIC thirdparty/nuked-opl3)
set_target_properties(nuked_opl3 PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED TRUE
)

set(TIMIDITY_SOURCES
    thirdparty/libtimidity/src/common.c
    thirdparty/libtimidity/src/instrum.c
    thirdparty/libtimidity/src/mix.c
    thirdparty/libtimidity/src/output.c
    thirdparty/libtimidity/src/playmidi.c
    thirdparty/libtimidity/src/readmidi.c
    thirdparty/libtimidity/src/resample.c
    thirdparty/libtimidity/src/stream.c
    thirdparty/libtimidity/src/tables.c
    thirdparty/libtimidity/src/timidity.c
)
add_library(timidity STATIC ${TIMIDITY_SOURCES})
target_include_directories(timidity PUBLIC thirdparty/libtimidity/src)
target_compile_definitions(timidity PRIVATE
    TIMIDITY_BUILD=1
    TIMIDITY_STATIC=1
    _CRT_SECURE_NO_WARNINGS
)
set_target_properties(timidity PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED TRUE)

set(JILL_SOURCES
    CMFOPL.C
    CONFIG.C
    COPYFILE.C
    DESIGN.C
    GAMECTRL.C
    GR.C
    GRASM.C
    HOSTAUDIO.C
    HOSTSDL.C
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

set(JILL_PLATFORM_SOURCES)
if(WIN32)
    list(APPEND JILL_PLATFORM_SOURCES JILL.RC)
endif()

add_executable(jill1 WIN32 ${JILL_SOURCES} ${JILL_PLATFORM_SOURCES})
target_compile_features(jill1 PRIVATE c_std_11)

# Host-only decoder check.  It is excluded from the normal game build but
# lets the vendored CMF/OPL path be exercised without opening an audio device.
add_executable(jill_cmf_validate EXCLUDE_FROM_ALL
    tools/CMF_VALIDATE.C
    CMFOPL.C
)
set_source_files_properties(tools/CMF_VALIDATE.C CMFOPL.C PROPERTIES LANGUAGE C)
target_compile_features(jill_cmf_validate PRIVATE c_std_11)
target_link_libraries(jill_cmf_validate PRIVATE nuked_opl3)

add_executable(jill_sfx_validate EXCLUDE_FROM_ALL
    tools/SFX_VALIDATE.C
    HOSTAUDIO.C
    CMFOPL.C
)
set_source_files_properties(tools/SFX_VALIDATE.C PROPERTIES LANGUAGE C)
target_compile_features(jill_sfx_validate PRIVATE c_std_11)
target_include_directories(jill_sfx_validate PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(jill_sfx_validate PRIVATE
    _CRT_SECURE_NO_WARNINGS
    TIMIDITY_STATIC=1
)
target_link_libraries(jill_sfx_validate PRIVATE
    SDL2::SDL2-static
    OpenAL::OpenAL
    nuked_opl3
    timidity
)

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
        TIMIDITY_STATIC=1
    )
    target_link_libraries(jill1 PRIVATE
        SDL2::SDL2main
        SDL2::SDL2-static
        OpenAL::OpenAL
        nuked_opl3
        timidity
    )
    target_compile_options(jill_cmf_validate PRIVATE /TC /W4)
    target_compile_definitions(jill_cmf_validate PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(jill1 PRIVATE -m32 -Wall -Wextra -Wpedantic)
    target_link_options(jill1 PRIVATE -m32)
    target_link_libraries(jill1 PRIVATE
        SDL2::SDL2main
        SDL2::SDL2-static
        OpenAL::OpenAL
        nuked_opl3
        timidity
    )
endif()
