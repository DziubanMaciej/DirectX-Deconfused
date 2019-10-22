# Add sources
set(TARGET_NAME ${DXD_TARGET_CORE})
add_subdirectories()
collect_sources(SOURCES ${DXD_TARGET_CORE})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# Target agnostic definitions
include_directories(. ${DXD_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/ExternalLibraries)
add_definitions(/MP -DDXD_COMPILE)
add_definitions_for_paths()
set_output_directories()
set_link_directory_to_lib()

# Core library
add_library (${DXD_TARGET_CORE} OBJECT ${SOURCES})
set_property(TARGET ${DXD_TARGET_CORE} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_shaders_properties(${DXD_TARGET_CORE} OFF)

# Dynamic library (DLL)
add_library(${DXD_TARGET_DLL} SHARED $<TARGET_OBJECTS:${DXD_TARGET_CORE}>)
target_link_libraries(${DXD_TARGET_DLL} ${DXD_LIBRARY_DEPENDENCIES})
set_target_properties(${DXD_TARGET_DLL} PROPERTIES OUTPUT_NAME ${DXD_DLL_NAME})

# Static library (LIB)
add_library(${DXD_TARGET_LIB} STATIC $<TARGET_OBJECTS:${DXD_TARGET_CORE}>)
set_target_properties(${DXD_TARGET_LIB} PROPERTIES OUTPUT_NAME ${DXD_LIB_NAME})

# Folders in solution
set_target_properties(${DXD_TARGET_CORE} PROPERTIES FOLDER DXD)
set_target_properties(${DXD_TARGET_DLL}  PROPERTIES FOLDER DXD)
set_target_properties(${DXD_TARGET_LIB}  PROPERTIES FOLDER DXD)
