#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <filesystem>
#include "GraphicsHelpers.h"

class CMaterial
{
public:
	
	//-------------------------------------
	// Construction / Usage
	//-------------------------------------

	// Main Constructor
	// Requires a vector of filemaps
	// Formats: NAME_RESOLUTION_TYPE.EXTENTION
	// Types supported: Albedo,AmbientOccusion,Displacement,Roughness,Metallness
	// It will set automatically the correct shaders depending on the use (Normals = PBR / No Normals = PBRNoNormals)
	CMaterial(std::vector<std::string> fileMaps);

	// Copy Constuctor
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
	auto& Texture() { return mPbrMaps.Albedo; }
	auto& TextureSRV() { return mPbrMaps.AlbedoSRV; }
	auto GetPtrVertexShader() { return mVertexShader; }
	auto GetPtrPixelShader() { return mPixelShader; }

	~CMaterial();

	void Release();

private:

	std::vector<std::string> mMapsStr;

	bool mHasNormals;

	//for regular models
	ID3D11VertexShader* mVertexShader;
	ID3D11GeometryShader* mGeometryShader; //WIP
	ID3D11PixelShader* mPixelShader;

	// All the pbr related maps that a model can have
	struct sPbrMaps
	{
		ID3D11Resource* Albedo;
		ID3D11ShaderResourceView* AlbedoSRV;
		ID3D11Resource* AO;
		ID3D11ShaderResourceView* AoSRV;
		ID3D11Resource* Displacement;
		ID3D11ShaderResourceView* DisplacementSRV;
		ID3D11Resource* Normal;
		ID3D11ShaderResourceView* NormalSRV;
		ID3D11Resource* Roughness;
		ID3D11ShaderResourceView* RoughnessSRV;
		ID3D11Resource* Metalness;
		ID3D11ShaderResourceView* MetalnessSRV;

	} mPbrMaps;

	void LoadMaps(std::vector<std::string>& maps);
};
