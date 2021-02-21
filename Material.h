#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <filesystem>
#include "GraphicsHelpers.h"

class CMaterial
{
public:

	CMaterial(std::string diffuseMap, std::string vertexShader, std::string pixelShader);

	CMaterial(std::vector<std::string> fileMaps, std::string vs, std::string ps);

	CMaterial(CMaterial& m);

	bool HasNormals() { return mHasNormals; }
	bool IsPbr() { return mIsPbr; }

	void RenderMaterial(bool basicGeometry = false);

	auto GetTexture() { return mPbrMaps.Albedo; }
	auto GetTextureSRV() { return mPbrMaps.AlbedoSRV; }

	~CMaterial();

private:
	
	std::vector<std::string> mMapsStr;

	std::string mVsStr;
	std::string mPsStr;

	bool mHasNormals;

	bool mIsPbr;

	//for regular models
	ID3D11VertexShader*		mVertexShader;
	ID3D11GeometryShader*   mGeometryShader; //WIP
	ID3D11PixelShader*		mPixelShader;

	//All the pbr related maps that a model can have
	struct sPbrMaps
	{
		ID3D11Resource*				Albedo;
		ID3D11ShaderResourceView*	AlbedoSRV;
		ID3D11Resource*				AO;
		ID3D11ShaderResourceView*	AoSRV;
		ID3D11Resource*				Displacement;
		ID3D11ShaderResourceView*	DisplacementSRV;
		ID3D11Resource*				Normal;
		ID3D11ShaderResourceView*	NormalSRV;
		ID3D11Resource*				Roughness;
		ID3D11ShaderResourceView*	RoughnessSRV;
	} mPbrMaps;

	void LoadMaps(std::vector<std::string>& maps);

};

