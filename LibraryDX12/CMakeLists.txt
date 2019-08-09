add_subdirectories()

# Compile options
add_definitions(/MP -DMAKEDLL)
add_definitions_for_shaders_path(${TARGET_NAME})
include_directories(. ${DXD_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/ExternalLibraries)

# Source files
collect_sources(SOURCES ${TARGET_NAME})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
set_output_directories()

# Target definition
set_link_directory_to_lib()
add_library (${TARGET_NAME} SHARED ${SOURCES})
set_working_directory_to_bin(${TARGET_NAME})
set_shaders_properties(${TARGET_NAME} OFF)

# Dependencies
target_link_libraries(${TARGET_NAME} "DXGI.lib" "D3D12.lib" "D3Dcompiler.lib" "DirectXTex.lib")
add_dependencies(${TARGET_NAME} "CopyResources" "DirectXTex")
