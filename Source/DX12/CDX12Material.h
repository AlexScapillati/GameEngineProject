#pragma once
#include <vector>

#include "CDX12Common.h"
#include "DX12Engine.h"

class CDX12Material
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
	CDX12Material(std::vector<std::string> fileMaps, CDX12Engine* engine);

	// Copy Constuctor
	// Deep Copy
	CDX12Material(CDX12Material& m);

	~CDX12Material() = default;

	// Return if the material has a normal map
	bool HasNormals() const { return mHasNormals; }

	// Set the shaders
	// Set the maps to the shader
	// Optionally decide to set depth only shaders
	void RenderMaterial(bool basicGeometry = false);

	// This two functions will change the shaders to set at rendering time
	void SetVertexShader(SShader& s) { mVertexShader = s; }
	void SetPixelShader(SShader& s) { mPixelShader = s; }

	//-------------------------------------
	// Data Access
	//-------------------------------------

	auto TextureFileName() { return mMapsStr.front(); }
	auto Texture() const { return mPbrMaps.Albedo.Get(); }
	auto TextureSRV() const { return mPbrMaps.AlbedoSRV.Get(); }
	auto GetPtrVertexShader() const { return mVertexShader; }
	auto GetPtrPixelShader() const { return mPixelShader; }

private:

	CDX12Engine* mEngine;

	std::vector<std::string> mMapsStr;

	bool mHasNormals;

	//for regular models
	SShader mVertexShader;
	SShader mGeometryShader; //WIP
	SShader mPixelShader;

	// All the pbr related maps that a model can have
	struct sPbrMaps
	{
		Resource Albedo;
		Resource AlbedoSRV;
		Resource AO;
		Resource AoSRV;
		Resource Displacement;
		Resource DisplacementSRV;
		Resource Normal;
		Resource NormalSRV;
		Resource Roughness;
		Resource RoughnessSRV;
		Resource Metalness;
		Resource MetalnessSRV;

	} mPbrMaps;

	void LoadMaps(std::vector<std::string>& maps);
};


