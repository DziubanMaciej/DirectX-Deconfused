add_subdirectories()

include_directories(.)
collect_sources(SOURCES ${TARGET_NAME})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
set_output_directories()

add_executable (${TARGET_NAME} WIN32 ${SOURCES})
target_link_libraries(${TARGET_NAME} "DXGI.lib" "D3D12.lib" "D3Dcompiler.lib")
set_working_directory_to_bin(${TARGET_NAME})
