#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <filesystem>

#include "DX11Engine.h"

class CMaterial
{
public:
	
	//-------------------------------------
	// Construction / Usage
	//-------------------------------------
	
	// Requires a vector of filemaps
	// Formats: NAME_RESOLUTION_TYPE.EXTENTION
	// Types supported: Albedo,AmbientOccusion,Displacement,Roughness,Metallness
	// It will set automatically the correct shaders depending on the use (Normals = PBR / No Normals = PBRNoNormals)
	CMaterial(std::vector<std::string> fileMaps, CDX11Engine* engine);
	
	// Deep Copy
	CMaterial(CMaterial& m);

	// Return if the material has a normal map
	bool HasNormals() { return mHasNormals; }

	// Set the shaders
	// Set the maps to the shader
	// Optionally decide to set depth only shaders
	void RenderMaterial(bool basicGeometry = false);

	// This two functions will change the shaders to set at rendering time
	void SetVertexShader(ID3D11VertexShader* s);
	void SetPixelShader(ID3D11PixelShader* s);
	
	//-------------------------------------
	// Data Access
	//-------------------------------------

	auto TextureFileName() { return mMapsStr.front(); }
	auto Texture() { return mPbrMaps.Albedo.Get(); }
	auto TextureSRV() { return mPbrMaps.AlbedoSRV.Get(); }
	auto GetPtrVertexShader() { return mVertexShader.Get(); }
	auto GetPtrPixelShader() { return mPixelShader.Get(); }

private:

	CDX11Engine* mEngine;

	std::vector<std::string> mMapsStr;

	bool mHasNormals;

	//for regular models
	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11GeometryShader> mGeometryShader; //WIP
	ComPtr<ID3D11PixelShader> mPixelShader;

	// All the pbr related maps that a model can have
	struct sPbrMaps
	{
		ComPtr<ID3D11Resource>           Albedo;
		ComPtr<ID3D11ShaderResourceView> AlbedoSRV;
		ComPtr<ID3D11Resource>           AO;
		ComPtr<ID3D11ShaderResourceView> AoSRV;
		ComPtr<ID3D11Resource>           Displacement;
		ComPtr<ID3D11ShaderResourceView> DisplacementSRV;
		ComPtr<ID3D11Resource>           Normal;
		ComPtr<ID3D11ShaderResourceView> NormalSRV;
		ComPtr<ID3D11Resource>           Roughness;
		ComPtr<ID3D11ShaderResourceView> RoughnessSRV;
		ComPtr<ID3D11Resource>           Metalness;
		ComPtr<ID3D11ShaderResourceView> MetalnessSRV;
	};

	sPbrMaps mPbrMaps;

	void LoadMaps(std::vector<std::string>& maps);
};
