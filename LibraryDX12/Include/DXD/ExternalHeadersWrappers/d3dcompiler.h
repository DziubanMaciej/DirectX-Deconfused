#pragma once

#include "DXD/ExternalHeadersWrappers/windows.h"
#include <wrl.h>
#include <d3dcompiler.h>

using ID3DBlobPtr = Microsoft::WRL::ComPtr<ID3DBlob>;
