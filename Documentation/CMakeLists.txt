set_output_directories()
add_custom_target (${TARGET_NAME})

set(DOXYFILE_NAME "Doxyfile.txt")
set(LOGOFILE_NAME "logo.png")
set(DOC_DIR ${CMAKE_BINARY_DIR}/doc)
set(API_DIRECTORY ${PROJECT_SOURCE_DIR}"/LibraryDX12/Include/DXD")

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
	COMMAND echo Creating doc directory - ${DOC_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_DIR}

	COMMAND echo Copying ${DOXYFILE_NAME} to doc directory
	COMMAND ${CMAKE_COMMAND} -E copy ${DOXYFILE_NAME} ${DOC_DIR}

	COMMAND echo Copying ${LOGOFILE_NAME} to doc directory
	COMMAND ${CMAKE_COMMAND} -E copy ${LOGOFILE_NAME} ${DOC_DIR}

	COMMAND echo Copying files from ${API_DIRECTORY} to doc directory
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${API_DIRECTORY} ${DOC_DIR}

	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
	COMMAND echo Generating documentation
	COMMAND doxygen.exe ${DOXYFILE_NAME}
	WORKING_DIRECTORY ${DOC_DIR}
)
