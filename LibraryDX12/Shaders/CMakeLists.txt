add_shaders_and_cmake_file(${TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel.hlsl                 Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_normal.hlsl          Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_normal_texture.hlsl  Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_post_process_black_bars.hlsl Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_post_process_convolution.hlsl   Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_post_process_linear_color_correction.hlsl   Pixel

    ${CMAKE_CURRENT_SOURCE_DIR}/vertex.hlsl                Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_normal.hlsl         Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_normal_texture.hlsl Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_normal_texture_sm.hlsl Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_normal_sm.hlsl      Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/vertex_post_process.hlsl   Vertex
)
