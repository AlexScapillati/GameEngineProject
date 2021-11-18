#pragma once

#include <atlconv.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

#include <queue>
#include <stdexcept>
#include <string>
#include <wrl/client.h>

#include <DirectXMath.h>
#include "..\Math/CVector2.h"
#include "..\Math/CVector3.h"
#include "..\Math/CVector4.h"

#include <queue>
#include <stdexcept>


using namespace Microsoft::WRL;

struct CommandAllocatorEntry
{
	uint64_t fenceValue = 0;
	ComPtr<ID3D12CommandAllocator> commandAllocator;
};

using Resource = ComPtr<ID3D12Resource>;
using CommandListQueue = std::queue<ComPtr<ID3D12GraphicsCommandList2>>;
using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;

struct SShader
{
	ComPtr<ID3DBlob> mblob;

	void LoadShaderFromFile(std::string const fileName)
	{
		if (FAILED(D3DReadFileToBlob(ATL::CA2W((fileName + ".cso").c_str()), &mblob)))
		{
			throw std::runtime_error("Error Loading " + fileName);
		}
	}
};
