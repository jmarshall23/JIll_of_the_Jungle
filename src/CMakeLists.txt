cmake_minimum_required(VERSION 3.21)

project(jill_recovered VERSION 0.1 LANGUAGES C CXX)

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

function(add_jill_episode target_name episode_define output_directory)
    add_executable(${target_name} WIN32 ${JILL_SOURCES} ${JILL_PLATFORM_SOURCES})
    target_compile_features(${target_name} PRIVATE c_std_11)
    target_compile_definitions(${target_name} PRIVATE ${episode_define}=1)

    # Keep each executable beside that episode's original data files.  The
    # debugger working directory matches the runtime output directory so a
    # launch from Visual Studio finds the same data as a direct launch.
    set_target_properties(${target_name} PROPERTIES
        OUTPUT_NAME "${target_name}"
        RUNTIME_OUTPUT_DIRECTORY "${output_directory}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${output_directory}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${output_directory}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${output_directory}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${output_directory}"
        VS_DEBUGGER_WORKING_DIRECTORY "${output_directory}"
        VS_GLOBAL_VcpkgEnabled "false"
        VS_GLOBAL_VcpkgApplocalDeps "false"
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /TC /W4 /wd4100 /wd4204)
        target_compile_definitions(${target_name} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            TIMIDITY_STATIC=1
        )
    else()
        target_compile_options(${target_name} PRIVATE -m32 -Wall -Wextra -Wpedantic)
        target_link_options(${target_name} PRIVATE -m32)
    endif()

    target_link_libraries(${target_name} PRIVATE
        SDL2::SDL2main
        SDL2::SDL2-static
        OpenAL::OpenAL
        nuked_opl3
        timidity
    )
endfunction()

add_jill_episode(jill1 JILL_EP1 "${CMAKE_CURRENT_SOURCE_DIR}/../ep1")
add_jill_episode(jill2 JILL_EP2 "${CMAKE_CURRENT_SOURCE_DIR}/../ep2")
add_jill_episode(jill3 JILL_EP3 "${CMAKE_CURRENT_SOURCE_DIR}/../ep3")

include(CTest)
if(BUILD_TESTING)
    add_test(NAME jill1_validate_intro
        COMMAND jill1 --validate intro.jn1)
    set_tests_properties(jill1_validate_intro PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../ep1")

    foreach(level_name IN ITEMS 0 1 3 9 10 11 13 15 17 20 INTRO)
        string(TOLOWER "${level_name}" level_test_name)
        add_test(NAME "jill2_validate_${level_test_name}"
            COMMAND jill2 --validate "${level_name}.JN2")
        set_tests_properties("jill2_validate_${level_test_name}" PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../ep2")
    endforeach()

    foreach(level_name IN ITEMS intro map 1 2 3 4 5 6 7 8 9 10 11 12 13 14)
        add_test(NAME "jill3_validate_${level_name}"
            COMMAND jill3 --validate "${level_name}.jn3")
        set_tests_properties("jill3_validate_${level_name}" PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../ep3")
    endforeach()
endif()

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

if(MSVC)
    target_compile_options(jill_cmf_validate PRIVATE /TC /W4)
    target_compile_definitions(jill_cmf_validate PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
