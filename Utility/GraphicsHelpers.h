//--------------------------------------------------------------------------------------
// Helper functions to unclutter and simplify main code (Scene.cpp/.h)
//--------------------------------------------------------------------------------------
// Code in .cpp file

#pragma once

#include "CMatrix4x4.h"
#include "../Common.h"
#include "../Shader.h"
#include <d3d11.h>
#include <atlbase.h> // C-string to unicode conversion function CA2CT

#include "MathHelpers.h"


//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

// Template function to update a constant buffer. Pass the DirectX constant buffer object and the C++ data structure
// you want to update it with. The structure will be copied in full over to the GPU constant buffer, where it will
// be available to shaders. This is used to update model and camera positions, lighting data etc.

inline void UpdateModelConstantBuffer(ID3D11Buffer* buffer, PerModelConstants& bufferData)
{
    D3D11_MAPPED_SUBRESOURCE cb;
    gD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
    memcpy(cb.pData, &bufferData, sizeof(PerModelConstants));
    gD3DContext->Unmap(buffer, 0);
}

inline void UpdateFrameConstantBuffer(ID3D11Buffer* buffer, PerFrameConstants& bufferData)
{
    D3D11_MAPPED_SUBRESOURCE cb;
    gD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
    
    memcpy(cb.pData, &bufferData, sizeof(PerFrameConstants));
    gD3DContext->Unmap(buffer, 0);
}

inline void UpdateLightBuffer(ID3D11Buffer* buffer, PerFrameLights& bufferData)
{
	D3D11_MAPPED_SUBRESOURCE cb;
    gD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

    //calculate the size of the frame constant buffer

    //remove the lights array from the equation
    auto size = sizeof(PerFrameLights) - sizeof(bufferData.lights);

    const auto sizeOfLightStruct = sizeof(sLight);
	
	//add just the lights used
	size += sizeOfLightStruct * bufferData.lights[0].numLights;

    memcpy(cb.pData, &bufferData, size);
    gD3DContext->Unmap(buffer, 0);
}

inline void UpdateSpotLightsBuffer(ID3D11Buffer* buffer, PerFrameSpotLights& bufferData)
{
	D3D11_MAPPED_SUBRESOURCE cb;
    gD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

    //calculate the size of the frame constant buffer

    //remove the lights array from the equation
    auto size = sizeof(PerFrameSpotLights) - sizeof(bufferData.spotLights);

    const auto sizeOfLightStruct = sizeof(sSpotLight);
	
	//add just the lights used
	size += sizeOfLightStruct * bufferData.spotLights[0].numLights;

    memcpy(cb.pData, &bufferData, size);
    gD3DContext->Unmap(buffer, 0);
}

inline void UpdateDirLightsBuffer(ID3D11Buffer* buffer, PerFrameDirLights& bufferData)
{
	D3D11_MAPPED_SUBRESOURCE cb;
    gD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

    //calculate the size of the frame constant buffer

    //remove the lights array from the equation
    auto size = sizeof(PerFrameDirLights) - sizeof(bufferData.dirLights);

    const auto sizeOfLightStruct = sizeof(sDirLights);
	
	//add just the lights used
	size += sizeOfLightStruct * bufferData.dirLights[0].numLights;

    memcpy(cb.pData, &bufferData, size);
    gD3DContext->Unmap(buffer, 0);
}

//--------------------------------------------------------------------------------------
// Texture Loading
//--------------------------------------------------------------------------------------

// Using Microsoft's open source DirectX Tool Kit (DirectXTK) to simplify file loading
// This function requires you to pass a ID3D11Resource* (e.g. &gTilesDiffuseMap), which manages the GPU memory for the
// texture and also a ID3D11ShaderResourceView* (e.g. &gTilesDiffuseMapSRV), which allows us to use the texture in shaders
// The function will fill in these pointers with usable data. Returns false on failure
bool LoadTexture(std::string filename, ID3D11Resource** texture, ID3D11ShaderResourceView** textureSRV);


//--------------------------------------------------------------------------------------
// Camera helpers
//--------------------------------------------------------------------------------------

// A "projection matrix" contains properties of a camera. Covered mid-module - the maths is an optional topic (not examinable).
// - Aspect ratio is screen width / height (like 4:3, 16:9)
// - FOVx is the viewing angle from left->right (high values give a fish-eye look),
// - near and far clip are the range of z distances that can be rendered
CMatrix4x4 MakeProjectionMatrix(float aspectRatio = 4.0f / 3.0f, float FOVx = ToRadians(60),
                                float nearClip = 0.1f, float farClip = 10000.0f);

