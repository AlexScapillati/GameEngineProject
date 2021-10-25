//--------------------------------------------------------------------------------------
// Helper functions to unclutter and simplify code elsewhere
//--------------------------------------------------------------------------------------

#pragma once

#include "GraphicsHelpers.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "ScreenGrab.h"

//--------------------------------------------------------------------------------------
// Texture Loading
//--------------------------------------------------------------------------------------

// Using Microsoft's open source DirectX Tool Kit (DirectXTK) to simplify texture loading
// This function requires you to pass a ID3D11Resource* (e.g. &gTilesDiffuseMap), which manages the GPU memory for the
// texture and also a ID3D11ShaderResourceView* (e.g. &gTilesDiffuseMapSRV), which allows us to use the texture in shaders
// The function will fill in these pointers with usable data. Returns false on failure
bool LoadTexture(std::string filename, ID3D11Resource** texture, ID3D11ShaderResourceView** textureSRV)
{

	auto res = false;

	filename = gMediaFolder + filename;

	// DDS files need a different function from other files
	std::string dds = ".dds"; // So check the filename extension (case insensitive)
	if (filename.size() >= 4 &&
		std::equal(dds.rbegin(), dds.rend(), filename.rbegin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }))
	{
		res = SUCCEEDED(DirectX::CreateDDSTextureFromFile(gD3DDevice.Get(), CA2CT(filename.c_str()), texture, textureSRV));
	}
	else
	{
		res = SUCCEEDED(DirectX::CreateWICTextureFromFile(gD3DDevice.Get(), gD3DContext.Get(), CA2CT(filename.c_str()), texture, textureSRV));
	}

	return res;
}

CVector3 GetTextureDimentions(ID3D11Resource* texture)
{
	ID3D11Texture2D* tex = nullptr;
	if (SUCCEEDED(texture->QueryInterface(&tex)))
	{
		D3D11_TEXTURE2D_DESC desc;
		tex->GetDesc(&desc);

		CVector3 dim;

		dim.x = static_cast<float>(desc.Width);
		dim.y = static_cast<float>(desc.Height);

		tex->Release();

		return dim;
	}

	return nullptr;
}


//--------------------------------------------------------------------------------------
// Camera Helpers
//--------------------------------------------------------------------------------------

// A "projection matrix" contains properties of a camera. Covered mid-module - the maths is an optional topic (not examinable).
// - Aspect ratio is screen width / height (like 4:3, 16:9)
// - FOVx is the viewing angle from left->right (high values give a fish-eye look),
// - near and far clip are the range of z distances that can be rendered
CMatrix4x4 MakeProjectionMatrix(float aspectRatio /*= 4.0f / 3.0f*/, float FOVx /*= ToRadians(60)*/,
	float nearClip /*= 0.1f*/, float farClip /*= 10000.0f*/)
{
	const auto tanFOVx = std::tan(FOVx * 0.5f);
	const auto scaleX = 1.0f / tanFOVx;
	const auto scaleY = aspectRatio / tanFOVx;
	const auto scaleZa = farClip / (farClip - nearClip);
	const auto scaleZb = -nearClip * scaleZa;

	return CMatrix4x4{ scaleX,   0.0f,    0.0f,   0.0f,
						 0.0f, scaleY,    0.0f,   0.0f,
						 0.0f,   0.0f, scaleZa,   1.0f,
						 0.0f,   0.0f, scaleZb,   0.0f };
}

CMatrix4x4 MakeOrthogonalMatrix(float width, float height, float nearClip, float farClip)
{

	auto scaleZa = 1 / (nearClip - farClip);
	auto scaleZb = nearClip / (nearClip - farClip);

	return CMatrix4x4
	{
		2 / width, 0.0f,     0.0f,      0.0f,
		0.0f,    2 / height, 0.0f,      0.0f,
		0.0f,    0.0f,		 scaleZa,   0.0f,
		0.0f,    0.0f,		 scaleZb,   1.0f };
}

bool SaveTextureToFile(ID3D11Resource* tex, std::string& fileName)
{
	std::wstring name(fileName.begin(), fileName.end());

	return SUCCEEDED(DirectX::SaveDDSTextureToFile(gD3DContext.Get(), tex, name.c_str()));
}