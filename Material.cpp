#include "Material.h"


#include "State.h"


extern std::string gMediaFolder;


CMaterial::CMaterial(std::string& diffuseMap, std::string& vertexShader, std::string& pixelShader)
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

CMaterial::CMaterial(std::vector<std::string> fileMaps, std::string& vs, std::string& ps)
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

	//Add the folder to the VS and PS fileNames

	ps = "Shaders\\" + ps;
	vs = "Shaders\\" + vs;


	//the model is pbr
	mIsPbr = true;

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

	if (mPixelShader)			mPixelShader->Release();			mPixelShader = nullptr;
	if (mVertexShader)			mVertexShader->Release();			mVertexShader = nullptr;
	if (mGeometryShader)		mGeometryShader->Release();			mGeometryShader = nullptr;

	if (mPbrMaps.AO) mPbrMaps.AO->Release();
	if (mPbrMaps.Normal) mPbrMaps.Normal->Release();
	if (mPbrMaps.Albedo) mPbrMaps.Albedo->Release();
	if (mPbrMaps.Displacement) mPbrMaps.Displacement->Release();
	if (mPbrMaps.Roughness) mPbrMaps.Roughness->Release();
	if (mPbrMaps.AoSRV) mPbrMaps.AoSRV->Release();
	if (mPbrMaps.NormalSRV) mPbrMaps.NormalSRV->Release();
	if (mPbrMaps.AlbedoSRV) mPbrMaps.AlbedoSRV->Release();
	if (mPbrMaps.DisplacementSRV) mPbrMaps.DisplacementSRV->Release();
	if (mPbrMaps.RoughnessSRV) mPbrMaps.RoughnessSRV->Release();


}

