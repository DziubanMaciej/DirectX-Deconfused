add_shaders_and_cmake_file(${TARGET_NAME}
    # Shadow map shaders
    ${CMAKE_CURRENT_SOURCE_DIR}/ShadowMap/normal_VS.hlsl         Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/ShadowMap/normal_texture_VS.hlsl Vertex

    # 3D shaders
    ${CMAKE_CURRENT_SOURCE_DIR}/3D/normal_PS.hlsl               Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/3D/normal_texture_PS.hlsl       Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/3D/normal_texture_VS.hlsl       Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/3D/normal_VS.hlsl               Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/3D/texture_normal_map_PS.hlsl   Vertex
    ${CMAKE_CURRENT_SOURCE_DIR}/3D/texture_normal_map_VS.hlsl   Pixel

    # Post process shaders
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/apply_bloom_PS.hlsl               Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/black_bars_PS.hlsl                Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/convolution_PS.hlsl               Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/gaussian_blur_CS.hlsl             Compute
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/gaussian_blur_PS.hlsl             Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/linear_color_correction_PS.hlsl   Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/VS.hlsl                           Vertex

    # Other shaders
    ${CMAKE_CURRENT_SOURCE_DIR}/generate_mip_maps_CS.hlsl   Compute
    ${CMAKE_CURRENT_SOURCE_DIR}/fog_PS.hlsl                 Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/lighting_PS.hlsl            Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/sprite_sampler_PS.hlsl      Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/ssao_PS.hlsl                Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/ssr_PS.hlsl                 Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/ssr_blur_PS.hlsl            Pixel
    ${CMAKE_CURRENT_SOURCE_DIR}/ssr_merge_PS.hlsl           Pixel
)
