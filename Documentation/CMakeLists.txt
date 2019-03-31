set_output_directories()
add_custom_target (${TARGET_NAME})

# Doxygen log file
set(DOXYGEN_LOG_NAME "doxygen_log.txt")
set(LOG_REDIRECTION >${DOXYGEN_LOG_NAME} 2>&1)

# Input files: source, configuration file, logo
set(INPUT_DIR_API ${PROJECT_SOURCE_DIR}/LibraryDX12/Include/DXD)
set(INPUT_DIR_INTERNAL ${PROJECT_SOURCE_DIR}/LibraryDX12/)
set(DOXYFILE_NAME "Doxyfile.txt")
set(LOGOFILE_NAME "logo.png")

# Output directories
set(DOC_DIR_ROOT ${CMAKE_BINARY_DIR}/doc)
set(DOC_DIR_API ${DOC_DIR_ROOT}/api)
set(DOC_DIR_INTERNAL ${DOC_DIR_ROOT}/internal)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND echo DOC_DIR_ROOT=${DOC_DIR_ROOT}
    COMMAND echo Creating required directories
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_DIR_ROOT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_DIR_API}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_DIR_INTERNAL}

    COMMAND echo Copying ${DOXYFILE_NAME}...
    COMMAND ${CMAKE_COMMAND} -E copy ${DOXYFILE_NAME} ${DOC_DIR_API}
    COMMAND ${CMAKE_COMMAND} -E copy ${DOXYFILE_NAME} ${DOC_DIR_INTERNAL}

    COMMAND echo Copying ${LOGOFILE_NAME}...
    COMMAND ${CMAKE_COMMAND} -E copy ${LOGOFILE_NAME} ${DOC_DIR_API}
    COMMAND ${CMAKE_COMMAND} -E copy ${LOGOFILE_NAME} ${DOC_DIR_INTERNAL}

    COMMAND echo Copying files from ${INPUT_DIR_API} to ${DOC_DIR_API}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${INPUT_DIR_API} ${DOC_DIR_API}

    COMMAND echo Copying files from ${INPUT_DIR_INTERNAL} to ${DOC_DIR_INTERNAL}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${INPUT_DIR_INTERNAL} ${DOC_DIR_INTERNAL}

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND echo Generating api documentation, log redirected to ${DOXYGEN_LOG_NAME}
    COMMAND doxygen.exe ${DOXYFILE_NAME} ${LOG_REDIRECTION}
    WORKING_DIRECTORY ${DOC_DIR_API}
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND echo Generating internal documentation, log redirected to ${DOXYGEN_LOG_NAME}
    COMMAND doxygen.exe ${DOXYFILE_NAME} ${LOG_REDIRECTION}
    WORKING_DIRECTORY ${DOC_DIR_INTERNAL}
)
