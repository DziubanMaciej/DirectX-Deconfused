add_shaders_and_cmake_file(${TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex.hlsl  Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_normal.hlsl  VertexNormal
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_normal_sm.hlsl  VertexNormalSM
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_post_process.hlsl  VertexPostProcess
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel.hlsl   Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_normal.hlsl   PixelNormal
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_post_process.hlsl   PixelPostProcess
)
