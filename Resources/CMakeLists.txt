set_output_directories()
add_custom_target (${TARGET_NAME})

set(resources_dir_name "Resources")
set(resources_dir_src ${PROJECT_SOURCE_DIR}/${resources_dir_name})
set(resources_dir_bin ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${resources_dir_name})

add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${resources_dir_src} ${resources_dir_bin}
    COMMAND ${CMAKE_COMMAND} -E remove ${resources_dir_bin}/CMakeLists.txt
)
