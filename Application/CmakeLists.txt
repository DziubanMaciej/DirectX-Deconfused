add_subdirectories()

# Compile options
add_definitions(/MP)
include_directories(. ${DXD_INCLUDE_DIR})

# Source files
collect_sources(SOURCES ${TARGET_NAME})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
set_output_directories()

# Target definition
add_executable (${TARGET_NAME} WIN32 ${SOURCES})
target_link_libraries(${TARGET_NAME} "${CMAKE_BINARY_DIR}/lib/Debug/LibraryDX12.lib")
set_working_directory_to_bin(${TARGET_NAME})
add_dependencies(${TARGET_NAME} "LibraryDX12")
