cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0012 NEW)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project("DXD")
include(CMakeMacros.cmake)
validate_parameters(CMAKE_BUILD_TYPE EXTERNAL_LIBS_BIN_PATH)

add_subdirectory("Application")
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${DEFAULT_APP})

set(TARGET_NAME "LibraryDX12")
add_subdirectory("LibraryDX12")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Development)

set(TARGET_NAME "Documentation")
add_subdirectory("Documentation")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Miscellaneous)
