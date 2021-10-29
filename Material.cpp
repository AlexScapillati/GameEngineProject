#include "Material.h"

extern std::string gMediaFolder;

CMaterial::CMaterial(std::vector<std::string> fileMaps, CDX11Engine* engine)
{
	mEngine = engine;

	mVertexShader = nullptr;
	mGeometryShader = nullptr;
	mPixelShader = nullptr;

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
		mVertexShader = mEngine->mPbrNormalVertexShader.Get();
		mPixelShader = mEngine->mPbrNormalPixelShader.Get();
	}
	else
	{
		mVertexShader = mEngine->mPbrVertexShader.Get();
		mPixelShader = mEngine->mPbrPixelShader.Get();
	}
}

CMaterial::CMaterial(CMaterial& m)
{
	mEngine = m.mEngine;

	mVertexShader = nullptr;
	mGeometryShader = nullptr;
	mPixelShader = nullptr;

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

void CMaterial::RenderMaterial(bool basicGeometry)
{
	ID3D11ShaderResourceView* nullSRV = nullptr;

	// Set Vertex Shader
	mEngine->GetContext()->VSSetShader(mVertexShader.Get(), nullptr, 0);

	//if the object is required rendered without effects or textures
	if (basicGeometry)
	{
		// Send Albedo map (in the aplha channel there is the opacity map)
		mEngine->GetContext()->PSSetShaderResources(0, 1, mPbrMaps.AlbedoSRV.GetAddressOf());

		// Use special depth-only rendering shaders
		if (HasNormals())
			mEngine->GetContext()->PSSetShader(mEngine->mDepthOnlyNormalPixelShader.Get(), nullptr, 0);
		else
			mEngine->GetContext()->PSSetShader(mEngine->mDepthOnlyPixelShader.Get(), nullptr, 0);

		// States - no blending, normal depth buffer and culling
		mEngine->GetContext()->OMSetBlendState(mEngine->mNoBlendingState, nullptr, 0xffffff);
		mEngine->GetContext()->OMSetDepthStencilState(mEngine->mUseDepthBufferState, 0);
	}
	else
	{
		// Set Pixel Shader
		mEngine->GetContext()->PSSetShader(mPixelShader.Get(), nullptr, 0);

		//Set Albedo map
		mEngine->GetContext()->PSSetShaderResources(0, 1, mPbrMaps.AlbedoSRV.GetAddressOf());

		//************************
		// Send PBR Maps
		//************************

		if (mPbrMaps.AO)
		{
			mEngine->GetContext()->PSSetShaderResources(1, 1, mPbrMaps.AoSRV.GetAddressOf());
			gPerModelConstants.hasAoMap = 1.0f;
		}
		else
		{
			mEngine->GetContext()->PSSetShaderResources(1, 1, &nullSRV);
			gPerModelConstants.hasAoMap = 0.0f;
		}

		if (mPbrMaps.Displacement)
		{
			mEngine->GetContext()->PSSetShaderResources(2, 1, mPbrMaps.DisplacementSRV.GetAddressOf());
		}
		else
		{
			mEngine->GetContext()->PSSetShaderResources(2, 1, &nullSRV);
		}

		if (mPbrMaps.Normal)
		{
			mEngine->GetContext()->PSSetShaderResources(3, 1, mPbrMaps.NormalSRV.GetAddressOf());
		}
		else
		{
			mEngine->GetContext()->PSSetShaderResources(3, 1, &nullSRV);
		}

		if (mPbrMaps.Roughness)
		{
			mEngine->GetContext()->PSSetShaderResources(4, 1, mPbrMaps.RoughnessSRV.GetAddressOf());
			gPerModelConstants.hasRoughnessMap = 1.0f;
		}
		else
		{
			mEngine->GetContext()->PSSetShaderResources(4, 1, &nullSRV);
			gPerModelConstants.hasRoughnessMap = 0.0f;
		}

		if (mPbrMaps.Metalness)
		{
			mEngine->GetContext()->PSSetShaderResources(5, 1, mPbrMaps.MetalnessSRV.GetAddressOf());
			gPerModelConstants.hasMetallnessMap = 1.0f;
		}
		else
		{
			
			mEngine->GetContext()->PSSetShaderResources(5, 1, &nullSRV);
			gPerModelConstants.hasMetallnessMap = 0.0f;
		}

	}
}

void CMaterial::SetVertexShader(ID3D11VertexShader* s)
{
	mVertexShader = s;
}

void CMaterial::SetPixelShader(ID3D11PixelShader* s)
{
	mPixelShader = s;
}

void CMaterial::LoadMaps(std::vector<std::string>& fileMaps)
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
		if (!mEngine->LoadTexture(fileMaps[0], mPbrMaps.Albedo.GetAddressOf(), mPbrMaps.AlbedoSRV.GetAddressOf()))
		{
			throw std::runtime_error("Error Loading: " + fileMaps[0]);
		}
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
				//found albedo map
				if (!mEngine->LoadTexture(originalFileName, mPbrMaps.Albedo.GetAddressOf(), mPbrMaps.AlbedoSRV.GetAddressOf()))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("Roughness") != std::string::npos)
			{
				//roughness map
				if (!mEngine->LoadTexture(originalFileName, mPbrMaps.Roughness.GetAddressOf(), mPbrMaps.RoughnessSRV.GetAddressOf()))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("AO") != std::string::npos)
			{
				//ambient occlusion map
				if (!mEngine->LoadTexture(originalFileName, mPbrMaps.AO.GetAddressOf(), mPbrMaps.AoSRV.GetAddressOf()))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("Displacement") != std::string::npos)
			{
				//found displacement map
				if (!mEngine->LoadTexture(originalFileName, mPbrMaps.Displacement.GetAddressOf(), mPbrMaps.DisplacementSRV.GetAddressOf()))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("Normal") != std::string::npos)
			{
				//TODO include LOD
				//
				//normal map
				if (!mEngine->LoadTexture(originalFileName, mPbrMaps.Normal.GetAddressOf(), mPbrMaps.NormalSRV.GetAddressOf()))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}

				mHasNormals = true;
			}
			else if (fileName.find("Metalness") != std::string::npos)
			{
				// Metallness Map
				if (!mEngine->LoadTexture(originalFileName, mPbrMaps.Metalness.GetAddressOf(), mPbrMaps.MetalnessSRV.GetAddressOf()))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
		}
	}
}