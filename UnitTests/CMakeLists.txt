# Compile options
add_definitions(/MP)
include_directories(. ${DXD_SRC_DIR} ${DXD_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/ExternalLibraries/gtest/googletest/include)
set_output_directories()
set_link_directory_to_lib()

# Get Sources
set(SOURCES main.cpp)

# Target definition
set(TARGET_NAME UnitTests)
add_executable (${TARGET_NAME} ${SOURCES})
set_working_directory_to_bin(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} ${DXD_LIB_NAME}.lib ${DXD_LIBRARY_DEPENDENCIES} gtestd.lib gmockd.lib)

add_definitions(-DDXD_STATIC_LINK)
add_dependencies(${TARGET_NAME} ${DXD_TARGET_LIB})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# Folders in solution
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Tests)
