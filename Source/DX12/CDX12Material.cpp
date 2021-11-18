#include "CDX12Material.h"

#include <stdexcept>

CDX12Material::CDX12Material(std::vector<std::string> fileMaps, CDX12Engine* engine)
{
	mEngine = engine;

	mHasNormals = false;

	mPbrMaps.Albedo = nullptr;
	mPbrMaps.AlbedoSRV = nullptr;
	mPbrMaps.AO = nullptr;
	mPbrMaps.AoSRV = nullptr;
	mPbrMaps.Displacement = nullptr;
	mPbrMaps.DisplacementSRV = nullptr;
	mPbrMaps.Normal = nullptr;
	mPbrMaps.NormalSRV = nullptr;
	mPbrMaps.Roughness = nullptr;
	mPbrMaps.RoughnessSRV = nullptr;

	mMapsStr = fileMaps;

	//load all the textures
	try
	{
		LoadMaps(mMapsStr);
	}
	catch (std::runtime_error e)
	{
		throw std::runtime_error(e.what());
	}

	// Set the shaders
	if (HasNormals())
	{
		mVertexShader = mEngine->mPbrNormalVertexShader;
		mPixelShader = mEngine->mPbrNormalPixelShader;
	}
	else
	{
		mVertexShader = mEngine->mPbrVertexShader;
		mPixelShader = mEngine->mPbrPixelShader;
	}
}

CDX12Material::CDX12Material(CDX12Material& m)
{
	mEngine = m.mEngine;
	
	mHasNormals = false;

	mPbrMaps.Albedo = nullptr;
	mPbrMaps.AlbedoSRV = nullptr;
	mPbrMaps.AO = nullptr;
	mPbrMaps.AoSRV = nullptr;
	mPbrMaps.Displacement = nullptr;
	mPbrMaps.DisplacementSRV = nullptr;
	mPbrMaps.Normal = nullptr;
	mPbrMaps.NormalSRV = nullptr;
	mPbrMaps.Roughness = nullptr;
	mPbrMaps.RoughnessSRV = nullptr;
	mPbrMaps.Metalness = nullptr;
	mPbrMaps.MetalnessSRV = nullptr;

	mMapsStr = m.mMapsStr;

	mPixelShader = m.mPixelShader;
	mGeometryShader = m.mGeometryShader;
	mVertexShader = m.mVertexShader;

	try
	{
		LoadMaps(mMapsStr);
	}
	catch (std::runtime_error e)
	{
		throw std::runtime_error(e.what());
	}
}

void CDX12Material::RenderMaterial(bool basicGeometry)
{
	Resource* nullSRV = nullptr;

	//// Set Vertex Shader
	//mEngine->GetContext()->VSSetShader(mVertexShader.Get(), nullptr, 0);

	////if the object is required rendered without effects or textures
	//if (basicGeometry)
	//{
	//	// Send Albedo map (in the aplha channel there is the opacity map)
	//	mEngine->GetContext()->PSSetShaderResources(0, 1, mPbrMaps.AlbedoSRV.GetAddressOf());

	//	// Use special depth-only rendering shaders
	//	if (HasNormals())
	//		mEngine->GetContext()->PSSetShader(mEngine->mDepthOnlyNormalPixelShader.Get(), nullptr, 0);
	//	else
	//		mEngine->GetContext()->PSSetShader(mEngine->mDepthOnlyPixelShader.Get(), nullptr, 0);

	//	// States - no blending, normal depth buffer and culling
	//	mEngine->GetContext()->OMSetBlendState(mEngine->mNoBlendingState.Get(), nullptr, 0xffffff);
	//	mEngine->GetContext()->OMSetDepthStencilState(mEngine->mUseDepthBufferState.Get(), 0);
	//}
	//else
	//{
	//	// Set Pixel Shader
	//	mEngine->GetContext()->PSSetShader(mPixelShader.Get(), nullptr, 0);

	//	//Set Albedo map
	//	mEngine->GetContext()->PSSetShaderResources(0, 1, mPbrMaps.AlbedoSRV.GetAddressOf());

	//	//************************
	//	// Send PBR Maps
	//	//************************
	//}
}


void CDX12Material::LoadMaps(std::vector<std::string>& fileMaps)
{
	// If the vector is empty
	if (fileMaps.empty())
	{
		// Throw an exception
		throw std::runtime_error("No maps found");
	}
	// If the vector is size 1,
	else if (fileMaps.size() == 1)
	{
		//assume it is an albedo map
		mPbrMaps.Albedo = mEngine->LoadTexture(fileMaps.at(0));
	}
	else
	{
		//for each file in the vector with the same name as the mesh one
		for (auto fileName : fileMaps)
		{
			auto originalFileName = fileName;

			//load it

			if (fileName.find("Albedo") != std::string::npos)
			{
				mPbrMaps.Albedo = mEngine->LoadTexture(originalFileName);
			}
			else if (fileName.find("Roughness") != std::string::npos)
			{
				//roughness map
				mPbrMaps.Roughness = mEngine->LoadTexture(originalFileName);
			}
			else if (fileName.find("AO") != std::string::npos)
			{
				//ambient occlusion map
				mPbrMaps.AO = mEngine->LoadTexture(originalFileName);
			}
			else if (fileName.find("Displacement") != std::string::npos)
			{
				//found displacement map
				mPbrMaps.Displacement = mEngine->LoadTexture(originalFileName);
			}
			else if (fileName.find("Normal") != std::string::npos)
			{
				//TODO include LOD
				//
				//normal map
				mPbrMaps.Normal = mEngine->LoadTexture(originalFileName);

				mHasNormals = true;
			}
			else if (fileName.find("Metalness") != std::string::npos)
			{
				// Metallness Map
				mPbrMaps.Metalness = mEngine->LoadTexture(originalFileName);
			}
		}
	}
}
