#pragma once

#include "ExternalHeadersWrappers/windows.h"
#include <wrl.h>
#include <d3dcompiler.h>

using ID3DBlobPtr = Microsoft::WRL::ComPtr<ID3DBlob>;
