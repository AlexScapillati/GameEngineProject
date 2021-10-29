//--------------------------------------------------------------------------------------
// Helper functions to unclutter and simplify main code (Scene.cpp/.h)
//--------------------------------------------------------------------------------------
// Code in .cpp file

#pragma once

#include "CMatrix4x4.h"
#include <d3d11.h>
#include <atlbase.h> // C-string to unicode conversion function CA2CT

#include "MathHelpers.h"
#include "..\DX11Engine.h"

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

// Template function to update a constant buffer. Pass the DirectX constant buffer object and the C++ data structure
// you want to update it with. The structure will be copied in full over to the GPU constant buffer, where it will
// be available to shaders. This is used to update model and camera positions, lighting data etc.

inline void CDX11Engine::UpdateModelConstantBuffer(ID3D11Buffer* buffer, PerModelConstants& bufferData) const
{
	D3D11_MAPPED_SUBRESOURCE cb;
	mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
	memcpy(cb.pData, &bufferData, sizeof(PerModelConstants));
	mD3DContext->Unmap(buffer, 0);
}

inline void CDX11Engine::UpdatePostProcessingConstBuffer(ID3D11Buffer* buffer, PostProcessingConstants& bufferData) const
{
	D3D11_MAPPED_SUBRESOURCE cb;
	mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
	memcpy(cb.pData, &bufferData, sizeof(PostProcessingConstants));
	mD3DContext->Unmap(buffer, 0);
}

inline void CDX11Engine::UpdateFrameConstantBuffer(ID3D11Buffer* buffer, PerFrameConstants& bufferData) const
{
	D3D11_MAPPED_SUBRESOURCE cb;
	mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

	memcpy(cb.pData, &bufferData, sizeof(PerFrameConstants));
	mD3DContext->Unmap(buffer, 0);
}

inline void CDX11Engine::UpdateLightConstantBuffer(ID3D11Buffer* buffer, PerFrameLights& bufferData, int numLights) const
{
	D3D11_MAPPED_SUBRESOURCE cb;
	mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

	//calculate the size of the frame constant buffer

	//remove the lights array from the equation
	auto size = sizeof(PerFrameLights) - sizeof(bufferData.lights);

	const auto sizeOfLightStruct = sizeof(sLight);

	//add just the lights used
	size += sizeOfLightStruct * numLights;

	memcpy(cb.pData, &bufferData, size);
	mD3DContext->Unmap(buffer, 0);
}

inline void CDX11Engine::UpdateSpotLightsConstantBuffer(ID3D11Buffer* buffer, PerFrameSpotLights& bufferData, int numLights) const
{
	D3D11_MAPPED_SUBRESOURCE cb;
	mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

	//calculate the size of the frame constant buffer

	//remove the lights array from the equation
	auto size = sizeof(PerFrameSpotLights) - sizeof(bufferData.spotLights);

	const auto sizeOfLightStruct = sizeof(sSpotLight);

	//add just the lights used
	size += sizeOfLightStruct * numLights;

	memcpy(cb.pData, &bufferData, size);
	mD3DContext->Unmap(buffer, 0);
}

inline void CDX11Engine::UpdateDirLightsConstantBuffer(ID3D11Buffer* buffer, PerFrameDirLights& bufferData, int numLights) const
{
	D3D11_MAPPED_SUBRESOURCE cb;
	mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

	//calculate the size of the frame constant buffer

	//remove the lights array from the equation
	auto size = sizeof(PerFrameDirLights) - sizeof(bufferData.dirLights);

	const auto sizeOfLightStruct = sizeof(sDirLights);

	//add just the lights used
	size += sizeOfLightStruct * numLights;

	memcpy(cb.pData, &bufferData, size);
	mD3DContext->Unmap(buffer, 0);
}

inline void CDX11Engine::UpdatePointLightsConstantBuffer(ID3D11Buffer* buffer, PerFramePointLights& bufferData, int numLights) const
{
	D3D11_MAPPED_SUBRESOURCE cb;
	mD3DContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

	//calculate the size of the frame constant buffer

	//remove the lights array from the equation
	auto size = sizeof(PerFramePointLights) - sizeof(bufferData.pointLights);

	const auto sizeOfLightStruct = sizeof(sPointLights);

	//add just the lights used
	size += sizeOfLightStruct * numLights;

	memcpy(cb.pData, &bufferData, size);
	mD3DContext->Unmap(buffer, 0);
}

//--------------------------------------------------------------------------------------
// Camera helpers
//--------------------------------------------------------------------------------------

// A "projection matrix" contains properties of a camera. Covered mid-module - the maths is an optional topic (not examinable).
// - Aspect ratio is screen width / height (like 4:3, 16:9)
// - FOVx is the viewing angle from left->right (high values give a fish-eye look),
// - near and far clip are the range of z distances that can be rendered
CMatrix4x4 MakeProjectionMatrix(float aspectRatio = 4.0f / 3.0f, float FOVx = ToRadians(60),
	float nearClip = 0.1f, float farClip = 10000.0f);

CMatrix4x4 MakeOrthogonalMatrix(float width, float height, float nearClip, float farClip);

