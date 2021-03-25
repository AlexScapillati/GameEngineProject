#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <filesystem>
#include "GraphicsHelpers.h"

class CMaterial
{
public:

	CMaterial(std::vector<std::string> fileMaps);

	CMaterial(CMaterial& m);

	//return if the material has a normal map
	bool HasNormals() { return mHasNormals; }

	void RenderMaterial(bool basicGeometry = false);

	void SetVertexShader(ID3D11VertexShader* s);

	void SetPixelShader(ID3D11PixelShader* s);

	auto GetTextureFileName() { return mMapsStr.front(); }
	auto GetTexture() { return mPbrMaps.Albedo; }
	auto GetTextureSRV() { return mPbrMaps.AlbedoSRV; }

	auto GetPtrVertexShader() { return mVertexShader; }

	auto GetPtrPixelShader() { return mPixelShader; }

	~CMaterial();

private:

	std::vector<std::string> mMapsStr;

	bool mHasNormals;

	//for regular models
	ID3D11VertexShader* mVertexShader;
	ID3D11GeometryShader* mGeometryShader; //WIP
	ID3D11PixelShader* mPixelShader;

	//All the pbr related maps that a model can have
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
