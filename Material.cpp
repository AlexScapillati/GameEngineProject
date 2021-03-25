#include "Material.h"

#include "State.h"

extern std::string gMediaFolder;

CMaterial::CMaterial(std::vector<std::string> fileMaps)
{
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
		mVertexShader = gPBRNormalVertexShader;
		mPixelShader = gPBRNormalPixelShader;
	}
	else
	{
		mVertexShader = gPBRVertexShader;
		mPixelShader = gPBRPixelShader;
	}
}

CMaterial::CMaterial(CMaterial& m)
{
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
	gD3DContext->VSSetShader(mVertexShader, nullptr, 0);

	//if the object is required rendered without effects or textures
	if (basicGeometry)
	{
		// Use special depth-only rendering shaders
		if (HasNormals())
			gD3DContext->PSSetShader(gDepthOnlyNormalPixelShader, nullptr, 0);
		else
			gD3DContext->PSSetShader(gDepthOnlyPixelShader, nullptr, 0);

		// States - no blending, normal depth buffer and culling
		gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
		gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	}
	else
	{
		// Set Pixel Shader
		gD3DContext->PSSetShader(mPixelShader, nullptr, 0);

		//Set Albedo map
		gD3DContext->PSSetShaderResources(0, 1, &mPbrMaps.AlbedoSRV);

		//************************
		// Send PBR Maps
		//************************

		if (mPbrMaps.AO)
		{
			gD3DContext->PSSetShaderResources(1, 1, &mPbrMaps.AoSRV);
			gPerModelConstants.hasAoMap = 1.0f;
		}
		else
		{
			gD3DContext->PSSetShaderResources(1, 1, &nullSRV);
			gPerModelConstants.hasAoMap = 0.0f;
		}

		if (mPbrMaps.Displacement)
		{
			gD3DContext->PSSetShaderResources(2, 1, &mPbrMaps.DisplacementSRV);
		}
		else
		{
			gD3DContext->PSSetShaderResources(2, 1, &nullSRV);
		}

		if (mPbrMaps.Normal)
		{
			gD3DContext->PSSetShaderResources(3, 1, &mPbrMaps.NormalSRV);
		}
		else
		{
			gD3DContext->PSSetShaderResources(3, 1, &nullSRV);
		}

		if (mPbrMaps.Roughness)
		{
			gD3DContext->PSSetShaderResources(4, 1, &mPbrMaps.RoughnessSRV);
			gPerModelConstants.hasRoughnessMap = 1.0f;
		}
		else
		{
			gD3DContext->PSSetShaderResources(4, 1, &nullSRV);
			gPerModelConstants.hasRoughnessMap = 0.0f;
		}

		if (mPbrMaps.Metalness)
		{
			gD3DContext->PSSetShaderResources(5, 1, &mPbrMaps.MetalnessSRV);
			gPerModelConstants.hasMetallnessMap = 1.0f;
		}
		else
		{
			
			gD3DContext->PSSetShaderResources(4, 1, &nullSRV);
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

CMaterial::~CMaterial()
{		
	if (mPbrMaps.AO				 != nullptr)				{mPbrMaps.AO->Release();				mPbrMaps.AO				 = nullptr;}
	if (mPbrMaps.Normal			 != nullptr)				{mPbrMaps.Normal->Release();			mPbrMaps.Normal			 = nullptr;}
	if (mPbrMaps.Albedo			 != nullptr)				{mPbrMaps.Albedo->Release();			mPbrMaps.Albedo			 = nullptr;}
	if (mPbrMaps.Displacement	 != nullptr)				{mPbrMaps.Displacement->Release();		mPbrMaps.Displacement	 = nullptr;}
	if (mPbrMaps.Roughness		 != nullptr)				{mPbrMaps.Roughness->Release();			mPbrMaps.Roughness		 = nullptr;}
	if (mPbrMaps.AoSRV			 != nullptr)				{mPbrMaps.AoSRV->Release();				mPbrMaps.AoSRV			 = nullptr;}
	if (mPbrMaps.NormalSRV		 != nullptr)				{mPbrMaps.NormalSRV->Release();			mPbrMaps.NormalSRV		 = nullptr;}
	if (mPbrMaps.AlbedoSRV		 != nullptr)				{mPbrMaps.AlbedoSRV->Release();			mPbrMaps.AlbedoSRV		 = nullptr;}
	if (mPbrMaps.DisplacementSRV != nullptr)				{mPbrMaps.DisplacementSRV->Release();	mPbrMaps.DisplacementSRV = nullptr;}
	if (mPbrMaps.RoughnessSRV	 != nullptr)				{mPbrMaps.RoughnessSRV->Release();		mPbrMaps.RoughnessSRV	 = nullptr;}
	if (mPbrMaps.Metalness		 != nullptr)				{mPbrMaps.Metalness->Release();			mPbrMaps.Metalness		 = nullptr;}
	if (mPbrMaps.MetalnessSRV	 != nullptr)				{mPbrMaps.MetalnessSRV->Release();		mPbrMaps.MetalnessSRV	 = nullptr;}
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
		if (!LoadTexture(fileMaps[0], &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV))
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
				if (!LoadTexture(originalFileName, &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("Roughness") != std::string::npos)
			{
				//roughness map
				if (!LoadTexture(originalFileName, &mPbrMaps.Roughness, &mPbrMaps.RoughnessSRV))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("AO") != std::string::npos)
			{
				//ambient occlusion map
				if (!LoadTexture(originalFileName, &mPbrMaps.AO, &mPbrMaps.AoSRV))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("Displacement") != std::string::npos)
			{
				//found displacement map
				//TODO: THERE IS A .EXR FILE THAT I DUNNO WHAT IS IT
				if (!LoadTexture(originalFileName, &mPbrMaps.Displacement, &mPbrMaps.DisplacementSRV))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
			else if (fileName.find("Normal") != std::string::npos)
			{
				//TODO include LOD
				//
				//normal map
				if (!LoadTexture(originalFileName, &mPbrMaps.Normal, &mPbrMaps.NormalSRV))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}

				mHasNormals = true;
			}
			else if (fileName.find("Metalness") != std::string::npos)
			{
				// Metallness Map
				if (!LoadTexture(originalFileName, &mPbrMaps.Metalness, &mPbrMaps.MetalnessSRV))
				{
					throw std::runtime_error("Error Loading: " + fileName);
				}
			}
		}
	}
}