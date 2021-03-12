#include "Material.h"

#include "State.h"

extern std::string gMediaFolder;

CMaterial::CMaterial(std::string diffuseMap, std::string vertexShader, std::string pixelShader)
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

	mMapsStr.push_back(diffuseMap);
	mPsStr = pixelShader;
	mVsStr = vertexShader;

	//Add the folder to the VS and PS fileNames

	pixelShader = "Shaders\\" + pixelShader;
	vertexShader = "Shaders\\" + vertexShader;

	//the model is not pbr
	mIsPbr = false;

	//just load the diffuse texture

	if (!LoadTexture(diffuseMap, &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV))
	{
		throw std::runtime_error("Error loading texture: " + diffuseMap);
	}

	// load the shaders

	if (!(mVertexShader = LoadVertexShader(vertexShader)))
	{
		throw std::runtime_error("error loading vertex shader");
	}

	if (!(mPixelShader = LoadPixelShader(pixelShader)))
	{
		throw std::runtime_error("error loading pixel shader");
	}
}

CMaterial::CMaterial(std::vector<std::string> fileMaps, std::string vs, std::string ps)
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
	mPsStr = ps;
	mVsStr = vs;

	//Add the folder to the VS and PS fileNames

	ps = "Shaders\\PBR_ps" /*+ ps*/;
	vs = "Shaders\\PBR_vs" /*+ vs*/;

	//the model is pbr
	mIsPbr = true;

	//load all the textures
	LoadMaps(fileMaps);

	// load the shaders
	if (!(mVertexShader = LoadVertexShader(vs)))
	{
		throw std::runtime_error("error loading vertex shader");
	}

	if (!(mPixelShader = LoadPixelShader(ps)))
	{
		throw std::runtime_error("error loading pixel shader");
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
	mPsStr = m.mPsStr;
	mVsStr = m.mVsStr;

	auto ps = mPsStr;
	auto vs = mVsStr;

	//Add the folder to the VS and PS fileNames

	ps = "Shaders\\" + ps;
	vs = "Shaders\\" + vs;

	if (m.mIsPbr)
	{
		//the model is pbr
		mIsPbr = false;

		//just load the diffuse texture

		if (!LoadTexture(mMapsStr[0], &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV))
		{
			throw std::runtime_error("Error loading texture: " + mMapsStr[0]);
		}

		// load the shaders
		if (!(mVertexShader = LoadVertexShader(vs)))
		{
			throw std::runtime_error("error loading vertex shader");
		}

		if (!(mPixelShader = LoadPixelShader(ps)))
		{
			throw std::runtime_error("error loading pixel shader");
		}
	}
	else
	{
		//the model is NOT pbr
		mIsPbr = false;

		//load the texture
		if (!(LoadTexture(mMapsStr.front(), &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV)))
		{
			throw std::runtime_error("Error loading texture");
		}

		// load the shaders
		if (!(mVertexShader = LoadVertexShader(vs)))
		{
			throw std::runtime_error("error loading vertex shader");
		}

		if (!(mPixelShader = LoadPixelShader(ps)))
		{
			throw std::runtime_error("error loading pixel shader");
		}
	}
}

void CMaterial::RenderMaterial(bool basicGeometry)
{
	//send everithing to the shaders

	//if the object is required rendered without effects or textures
	if (basicGeometry)
	{
		// Use special depth-only rendering shaders
		gD3DContext->VSSetShader(mVertexShader, nullptr, 0);

		//even thought we are not using normals we need to set the correct pixel shader
		if (mHasNormals)
		{
			gD3DContext->PSSetShader(gPbrDepthOnlyPixelShader, nullptr, 0);
		}
		else
		{
			gD3DContext->PSSetShader(gDepthOnlyPixelShader, nullptr, 0);
		}

		// States - no blending, normal depth buffer and culling
		gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
		gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
		gD3DContext->RSSetState(gCullBackState);
	}
	else
	{
		//check if the current vertex shader is not already set

		ID3D11VertexShader* VS = nullptr;

		gD3DContext->VSGetShader(&VS, nullptr, 0);

		if (VS != mVertexShader)
		{
			gD3DContext->VSSetShader(mVertexShader, nullptr, 0);
		}

		//same with the PS

		ID3D11PixelShader* PS = nullptr;

		gD3DContext->PSGetShader(&PS, nullptr, 0);

		if (PS != mPixelShader)
		{
			gD3DContext->PSSetShader(mPixelShader, nullptr, 0);
		}

		//same for the texture

		ID3D11ShaderResourceView* PSSRV = nullptr;

		gD3DContext->PSGetShaderResources(0, 1, &PSSRV);

		if (PSSRV != mPbrMaps.AlbedoSRV)
		{
			gD3DContext->PSSetShaderResources(0, 1, &mPbrMaps.AlbedoSRV);
		}

		//************************
		// Send PBR Maps
		//************************

		if (mPbrMaps.AO)
		{
			gD3DContext->PSSetShaderResources(1, 1, &mPbrMaps.AoSRV);
		}

		if (mPbrMaps.Displacement)
		{
			gD3DContext->PSSetShaderResources(2, 1, &mPbrMaps.DisplacementSRV);
		}

		//if this object has normals
		if (mPbrMaps.Normal)
		{
			gD3DContext->PSSetShaderResources(3, 1, &mPbrMaps.NormalSRV);
		}

		if (mPbrMaps.Roughness)
		{
			gD3DContext->PSSetShaderResources(4, 1, &mPbrMaps.RoughnessSRV);
		}
	}
}

CMaterial::~CMaterial()
{
	if (mPixelShader)				mPixelShader->Release();				mPixelShader = nullptr;
	if (mVertexShader)				mVertexShader->Release();				mVertexShader = nullptr;
	if (mGeometryShader)			mGeometryShader->Release();				mGeometryShader = nullptr;

	if (mPbrMaps.AO)				mPbrMaps.AO->Release();					mPbrMaps.AO = nullptr;
	if (mPbrMaps.Normal)			mPbrMaps.Normal->Release();				mPbrMaps.Normal = nullptr;
	if (mPbrMaps.Albedo)			mPbrMaps.Albedo->Release();				mPbrMaps.Albedo = nullptr;
	if (mPbrMaps.Displacement)		mPbrMaps.Displacement->Release();		mPbrMaps.Displacement = nullptr;
	if (mPbrMaps.Roughness)			mPbrMaps.Roughness->Release();			mPbrMaps.Roughness = nullptr;
	if (mPbrMaps.AoSRV)				mPbrMaps.AoSRV->Release();				mPbrMaps.AoSRV = nullptr;
	if (mPbrMaps.NormalSRV)			mPbrMaps.NormalSRV->Release();			mPbrMaps.NormalSRV = nullptr;
	if (mPbrMaps.AlbedoSRV)			mPbrMaps.AlbedoSRV->Release();			mPbrMaps.AlbedoSRV = nullptr;
	if (mPbrMaps.DisplacementSRV)	mPbrMaps.DisplacementSRV->Release();	mPbrMaps.DisplacementSRV = nullptr;
	if (mPbrMaps.RoughnessSRV)		mPbrMaps.RoughnessSRV->Release();		mPbrMaps.RoughnessSRV = nullptr;
}

void CMaterial::LoadMaps(std::vector<std::string>& fileMaps)
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
			mHasNormals = true;

			//TODO include LOD
			//
			//normal map
			if (!LoadTexture(originalFileName, &mPbrMaps.Normal, &mPbrMaps.NormalSRV))
			{
				throw std::runtime_error("Error Loading: " + fileName);
			}
		}
	}
}