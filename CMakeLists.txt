cmake_minimum_required (VERSION 2.6)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project("DXD")
include(CMakeMacros.cmake)

set(TARGET_NAME "Application")
add_subdirectory("Application")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Development)
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${TARGET_NAME})

set(TARGET_NAME "ApplicationDX12")
add_subdirectory("ApplicationDX12")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Development)

set(TARGET_NAME "Documentation")
add_subdirectory("Documentation")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Development)

set(TARGET_NAME "LibraryDX12")
add_subdirectory("LibraryDX12")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Development)
