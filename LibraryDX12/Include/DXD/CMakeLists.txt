add_sources_and_cmake_file(${TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CallbackHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NonCopyableAndMovable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Window.h
)

add_subdirectories()
