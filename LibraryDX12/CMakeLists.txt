add_subdirectories()

# Compile options
add_definitions(/MP -DMAKEDLL)
add_definitions_for_shaders_path(${TARGET_NAME})
include_directories(. ${DXD_INCLUDE_DIR})

# Source files
collect_sources(SOURCES ${TARGET_NAME})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
set_output_directories()

# Target definition
add_library (${TARGET_NAME} SHARED ${SOURCES})
target_link_libraries(${TARGET_NAME} "DXGI.lib" "D3D12.lib" "D3Dcompiler.lib")
set_working_directory_to_bin(${TARGET_NAME})
add_dependencies(${TARGET_NAME} "CopyResources")
set_shaders_properties(${TARGET_NAME} OFF)
