#include "Material.h"


#include "State.h"


extern std::string gMediaFolder;


void GetSimilarFilesIn(std::string& dirPath, std::vector<std::string>& fileNames, std::string& fileToFind)
{
	//get the name of the file 
	auto esPos = fileToFind.find_last_of('_');
	std::string es;

	if (esPos != std::string::npos)
	{
		es = fileToFind.substr(0, esPos);

		//iterate through the directory
		std::filesystem::recursive_directory_iterator iter(dirPath);

		std::filesystem::recursive_directory_iterator end;

		while (iter != end)
		{
			if (!is_directory(iter->path()))
			{
				//get just the ID of the iterated file
				auto filename = iter->path().filename().string();

				//get the position of the first underscore that is after the name
				auto currIdPos = filename.find_first_of('_');

				if (currIdPos != std::string::npos)
				{
					//get the first name of the file
					auto fileNameS = filename.substr(0, currIdPos);

					//if this file id is the same as the one in the file to find
					if (fileNameS == es)
					{
						fileNames.push_back(iter->path().filename().string());
						iter.disable_recursion_pending();
					}
				}
			}
			std::error_code ec;
			iter.increment(ec);
			if (ec)
			{
				throw std::runtime_error("Error accessing " + ec.message());
			}
		}
	}
}



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


	//For PBR
	std::vector<std::string> files;

	//go thought the media folder and get the files with the same id
	GetSimilarFilesIn(gMediaFolder, files, diffuseMap);

	//if there were some
	if (!files.empty())
	{

		//the model is pbr
		mIsPbr = true;

		//for each file in the vector with the same name as the mesh one
		for (auto fileName : files)
		{
			auto originalFileName = fileName;

			//load it

			//get the type or first attribute of the map

			auto arrtPos = fileName.find_first_of('_') + 1;

			fileName = fileName.substr(arrtPos);

			//remove the extenstion

			fileName = fileName.substr(0, fileName.find_first_of('.'));

			if (fileName[0] == '8' || fileName[0] == '4' || fileName[0] == '2')
			{
				//map
				// TODO: ONLY SUPPORTED 8,4,2K MAPS
				// This should be fine since megascans only gives one resolution typer per model
				// TODO: include the other resolutions in the /Thumbs folder
				// TODO: MAKE A DISTINCTION (ARRAY OF TEXTURE TO SCALE) OR TO IMPLEMENT IN QUALITY SETTINGS?

				auto mapTypePos = fileName.find_first_of('_');

				if (mapTypePos != std::string::npos)
				{
					//check what type of texture it is

					fileName = fileName.substr(mapTypePos + 1);

					//remove the extenstion

					fileName = fileName.substr(0, fileName.find_last_of('.'));

					if (fileName == "Albedo")
					{
						//found albedo map
						if (!LoadTexture(originalFileName, &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV))
						{
							throw std::runtime_error("Error Loading: " + fileName);
						}
					}
					else if (fileName == "AO")
					{
						//ambient occlusion map
						if (!LoadTexture(originalFileName, &mPbrMaps.AO, &mPbrMaps.AoSRV))
						{
							throw std::runtime_error("Error Loading: " + fileName);
						}
					}
					else if (fileName == "Displacement")
					{
						//found displacement map
						//TODO: THERE IS A .EXR FILE THAT I DUNNO WHAT IS IT
						if (!LoadTexture(originalFileName, &mPbrMaps.Displacement, &mPbrMaps.DisplacementSRV))
						{
							throw std::runtime_error("Error Loading: " + fileName);
						}
					}
					//find rather than compare because there could be multiple normal maps because of the LOD
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
					else if (fileName == "Roughness")
					{
						//roughness map
						if (!LoadTexture(originalFileName, &mPbrMaps.Roughness, &mPbrMaps.RoughnessSRV))
						{
							throw std::runtime_error("Error Loading: " + fileName);
						}
					}
				}
			}
			else if (fileName[0] == 'L')
			{
				//Level of Detail

				//push the all the meshes avaliable in this vector

			}
			else
			{
				//TODO more functionality maybe?
				throw std::runtime_error("File not recognized");
			}
		}
	}
	else
	{
		//the model is not pbr
		mIsPbr = false;

		//just load the diffuse texture

		if (!LoadTexture(diffuseMap, &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV))
		{
			throw std::runtime_error("Error loading texture: " + diffuseMap);
		}
	}

	// load the shaders


	if (!(mVertexShader = LoadVertexShader(vertexShader)))
	{
		throw std::runtime_error("error loading vertex shader");
	}

	mPixelShader = LoadPixelShader(pixelShader);

	if (!(mPixelShader = LoadPixelShader(pixelShader)))
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
		if (mPbrMaps.Normal)
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

		//TODO remove 
		if (!mPbrMaps.Albedo)
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

