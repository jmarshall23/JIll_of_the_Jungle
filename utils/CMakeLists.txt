cmake_minimum_required(VERSION 3.15)

project(unlzexe VERSION 0.9 LANGUAGES C)

add_executable(unlzexe unlzexe.c)

set_target_properties(unlzexe PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# Multi-configuration generators normally append Debug/Release to the output
# directory. Keep every configuration in utils/bin as requested.
foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${config}" config_upper)
    set_target_properties(unlzexe PROPERTIES
        "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    )
endforeach()

if(MSVC)
    target_compile_definitions(unlzexe PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(unlzexe PRIVATE /W4)
else()
    target_compile_options(unlzexe PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING
        "This configuration produces a 64-bit executable. On Visual Studio, "
        "configure with '-A Win32' to build the requested 32-bit executable."
    )
endif()
