macro(add_documentation_target TARGET_NAME DOC_DIR SRC_DIR)
    # Constants
    set(DOXYGEN_LOG_NAME "doxygen_log.txt")
    set(DOXYFILE_NAME "Doxyfile.txt")
    set(LOGOFILE_NAME "logo.png")

    # Define custom target
    add_custom_target (${TARGET_NAME})
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Documentation)

    # Copy all required files to doc dir
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND echo Creating documentation of "${SRC_DIR}" in "${DOC_DIR}..."
        COMMAND echo Creating doc directory...
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_DIR}

        COMMAND echo Copying doxyfile and logo to doc directory...
        COMMAND ${CMAKE_COMMAND} -E copy ${DOXYFILE_NAME} ${DOC_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${LOGOFILE_NAME} ${DOC_DIR}

        COMMAND echo Copying source files to doc directory...
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_DIR} ${DOC_DIR}

        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Run doxygen to generate docs
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND echo Generating documentation...
        COMMAND doxygen.exe ${DOXYFILE_NAME} >${DOXYGEN_LOG_NAME} 2>&1
        COMMAND echo Done. Log redirected to ${DOXYGEN_LOG_NAME}.
        WORKING_DIRECTORY ${DOC_DIR}
    )
endmacro()

set(DOC_DIR_ROOT ${CMAKE_BINARY_DIR}/doc)
add_documentation_target(ApiDocumentation      ${DOC_DIR_ROOT}/api      ${DXD_SRC_DIR}/Include/DXD)
add_documentation_target(InternalDocumentation ${DOC_DIR_ROOT}/internal ${DXD_SRC_DIR})
