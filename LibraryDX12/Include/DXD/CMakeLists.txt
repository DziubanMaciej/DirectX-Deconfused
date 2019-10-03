add_sources_and_cmake_file(${TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CallbackHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Light.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Mesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Object.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Settings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Text.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Window.h
)

add_subdirectories()
