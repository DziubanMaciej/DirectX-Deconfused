# DirectX Texture Library
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#
# http://go.microsoft.com/fwlink/?LinkId=248926

cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WarningsLib "-Wall" "-Wpedantic" "-Wextra")
set(WarningsEXE ${WarningsLib} "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-switch" "-Wno-switch-enum" "-Wno-language-extension-token" "-Wno-missing-prototypes")

add_definitions(/MP)
set_output_directories()

add_library (${TARGET_NAME} STATIC
    DirectXTex/BC.h
    DirectXTex/BCDirectCompute.h
    DirectXTex/d3dx12.h
    DirectXTex/DDS.h
    DirectXTex/DirectXTex.h
    DirectXTex/DirectXTexP.h
    DirectXTex/filters.h
    DirectXTex/scoped.h
    DirectXTex/BC.cpp
    DirectXTex/BC4BC5.cpp
    DirectXTex/BC6HBC7.cpp
    DirectXTex/BCDirectCompute.cpp
    DirectXTex/DirectXTexCompress.cpp
    DirectXTex/DirectXTexCompressGPU.cpp
    DirectXTex/DirectXTexConvert.cpp
    DirectXTex/DirectXTexD3D11.cpp
    DirectXTex/DirectXTexD3D12.cpp
    DirectXTex/DirectXTexDDS.cpp
    DirectXTex/DirectXTexFlipRotate.cpp
    DirectXTex/DirectXTexHDR.cpp
    DirectXTex/DirectXTexImage.cpp
    DirectXTex/DirectXTexMipmaps.cpp
    DirectXTex/DirectXTexMisc.cpp
    DirectXTex/DirectXTexNormalMaps.cpp
    DirectXTex/DirectXTexPMAlpha.cpp
    DirectXTex/DirectXTexResize.cpp
    DirectXTex/DirectXTexTGA.cpp
    DirectXTex/DirectXTexUtil.cpp
    DirectXTex/DirectXTexWIC.cpp
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ExternalLibraries)
target_include_directories( ${TARGET_NAME} PUBLIC DirectXTex )
target_compile_options( ${TARGET_NAME} PRIVATE /fp:fast /Wall /permissive- /Zc:__cplusplus )

