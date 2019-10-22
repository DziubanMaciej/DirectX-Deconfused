# Directories
set(DXD_SRC_DIR ${PROJECT_SOURCE_DIR}/LibraryDX12)
set(DXD_INCLUDE_DIR ${DXD_SRC_DIR}/Include)

# Target names
set(DXD_TARGET_CORE "DXD_core")
set(DXD_TARGET_DLL  "DXD_dll")
set(DXD_TARGET_LIB  "DXD_lib")

# File names
set(DXD_DLL_NAME "DXD")
set(DXD_LIB_NAME "DXD_static")
set(DXD_LIBRARY_DEPENDENCIES "DXGI.lib" "D3D12.lib" "D3Dcompiler.lib" "d3d11.lib" "d2d1.lib" "dwrite.lib" "directxtex.lib")