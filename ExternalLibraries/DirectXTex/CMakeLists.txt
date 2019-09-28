# DirectX Texture Library
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#
# http://go.microsoft.com/fwlink/?LinkId=248926

cmake_minimum_required (VERSION 3.8)
project (DirectXTex_CMake LANGUAGES CXX)
include(${DXD_MACROS_PATH})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WarningsLib "-Wall" "-Wpedantic" "-Wextra")
set(WarningsEXE ${WarningsLib} "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic" "-Wno-switch" "-Wno-switch-enum" "-Wno-language-extension-token" "-Wno-missing-prototypes")

add_definitions(/MP)
set_output_directories_to_dxd()

add_library (directxtex STATIC
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

target_include_directories( directxtex PUBLIC DirectXTex )

target_compile_options( directxtex PRIVATE /fp:fast )

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options( directxtex PRIVATE ${WarningsLib} )
    target_compile_options( texassemble PRIVATE ${WarningsEXE}  )
    target_compile_options( texconv PRIVATE ${WarningsEXE}  )
    target_compile_options( texdiag PRIVATE ${WarningsEXE} "-Wno-double-promotion" )
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        target_compile_options( directxtex PRIVATE /arch:SSE2 )
        target_compile_options( texassemble PRIVATE /arch:SSE2 )
        target_compile_options( texconv PRIVATE /arch:SSE2 )
        target_compile_options( texdiag PRIVATE /arch:SSE2 )
    endif()
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( directxtex PRIVATE /Wall /permissive- /Zc:__cplusplus )
endif()
