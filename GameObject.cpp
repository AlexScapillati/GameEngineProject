//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.


#include "GameObject.h"
#include <codecvt>
#include "GraphicsHelpers.h"
#include "Shader.h"
#include <locale>
#include <filesystem>
#include <utility>

#include "State.h"


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

CGameObject::CGameObject(std::string mesh, std::string name, const std::string& diffuseMap, std::string&
	vertexShader, std::string& pixelShader, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/)
{

	if (mesh.empty()) throw std::exception("Error Loading Object");

	mName = std::move(name);

	//initialize graphics related components 

	mEnabled = true;

	mVertexShader = nullptr;
	mGeometryShader = nullptr;
	mPixelShader = nullptr;

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
	GetSimilarFilesIn(gMediaFolder, files, mesh);

	//if there were some
	if (!files.empty())
	{
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
				//import it later
				mMeshFiles.push_back(originalFileName);

			}
			else
			{
				//TODO more functionality maybe?
				throw std::runtime_error("File not recognized");
			}
		}


		//if this model has a normal map
		if (mPbrMaps.Normal)
		{
			try
			{
				//load the most detailed mesh with tangents required
				mMesh = new CMesh(mMeshFiles.front(), true);
			}
			catch (std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}
		else
		{
			try
			{
				mMesh = new CMesh(mMeshFiles.front() /* TODO .front for best resolution*/);
			}
			catch (std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}

		// Set default matrices from mesh //TODO could set initially the less detailed one and then load the more complex ones according to the camera position
		mWorldMatrices.resize(mMesh->NumberNodes());

		for (auto i = 0; i < mWorldMatrices.size(); ++i)
		{
			mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
		}

	}
	else
	{
		//default model
		//not PBR
		//that could be light models or cube maps
		try
		{
			mMesh = new CMesh(mesh);

			// Set default matrices from mesh
			mWorldMatrices.resize(mMesh->NumberNodes());
			for (auto i = 0; i < mWorldMatrices.size(); ++i)
				mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
		}
		catch (std::exception& e)
		{
			throw std::runtime_error(e.what());
		}

		//TODO

		if (!(mVertexShader = LoadVertexShader(vertexShader)))
		{
			throw std::runtime_error("error loading vertex shader");
		}

		mPixelShader = LoadPixelShader(pixelShader);

		if (!(mPixelShader = LoadPixelShader(pixelShader)))
		{
			throw std::runtime_error("error loading pixel shader");
		}

		if (!LoadTexture(diffuseMap, &mPbrMaps.Albedo, &mPbrMaps.AlbedoSRV))
		{
			throw std::runtime_error("Error loading texture: " + diffuseMap);
		}
	}

	//TODO remove
	mVertexShader = LoadVertexShader(vertexShader);

	if (!mVertexShader)
	{
		throw std::runtime_error("error loading vertex shader");
	}

	mPixelShader = LoadPixelShader(pixelShader);

	if (!mPixelShader)
	{
		throw std::runtime_error("error loading pixel shader");
	}

	//geometry loaded, set its position...

	SetPosition(position);
	SetRotation(rotation);
	SetScale(scale);
}



// The render function simply passes this model's matrices over to Mesh:Render.
// All other per-frame constants must have been set already along with shaders, textures, samplers, states etc.
void CGameObject::Render(bool basicGeometry)
{

	if (!mEnabled) return;

	//General rendering

	//gPerModelConstants.parallaxDepth = 0.006f; //TODO
	//
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

	//TODO render the the correct mesh according to the camera distance
	mMesh->Render(mWorldMatrices);
}

bool CGameObject::Update(float updateTime)
{
	return true; //TODO WIP
}

CGameObject::~CGameObject()
{

	delete mMesh;

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


// Control a given node in the model using keys provided. Amount of motion performed depends on frame time
void CGameObject::Control(int node, float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
	KeyCode turnCW, KeyCode turnCCW, KeyCode moveForward, KeyCode moveBackward)
{
	auto& matrix = mWorldMatrices[node]; // Use reference to node matrix to make code below more readable

	if (KeyHeld(turnUp))
	{
		matrix = MatrixRotationX(ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld(turnDown))
	{
		matrix = MatrixRotationX(-ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld(turnRight))
	{
		matrix = MatrixRotationY(ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld(turnLeft))
	{
		matrix = MatrixRotationY(-ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld(turnCW))
	{
		matrix = MatrixRotationZ(ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld(turnCCW))
	{
		matrix = MatrixRotationZ(-ROTATION_SPEED * frameTime) * matrix;
	}

	// Local Z movement - move in the direction of the Z axis, get axis from world matrix
	const auto localZDir = Normalise(matrix.GetRow(2)); // normalise axis in case world matrix has scaling
	if (KeyHeld(moveForward))
	{
		matrix.SetRow(3, matrix.GetRow(3) + localZDir * MOVEMENT_SPEED * frameTime);
	}
	if (KeyHeld(moveBackward))
	{
		matrix.SetRow(3, matrix.GetRow(3) - localZDir * MOVEMENT_SPEED * frameTime);
	}
}

// Getters - model only stores matrices. Position, rotation and scale are extracted if requested.

CVector3 CGameObject::Position(int node) { return mWorldMatrices[node].GetRow(3); }

// Position is on bottom row of matrix

CVector3 CGameObject::Rotation(int node) { return mWorldMatrices[node].GetEulerAngles(); }

// Getting angles from a matrix is complex - see .cpp file

inline CVector3 CGameObject::Scale(int node) {
	return { Length(mWorldMatrices[node].GetRow(0)),
		Length(mWorldMatrices[node].GetRow(1)),
		Length(mWorldMatrices[node].GetRow(2)) };
}

// Scale is length of rows 0-2 in matrix

CMatrix4x4 CGameObject::WorldMatrix(int node) { return mWorldMatrices[node]; }

float* CGameObject::DirectPosition()
{
	float* pos[] =
	{
		&mWorldMatrices[0].e30,
		&mWorldMatrices[0].e31,
		&mWorldMatrices[0].e32,
	};
	return *pos;
}


CMesh* CGameObject::GetMesh() const { return mMesh; }

// Setters - model only stores matricies , so if user sets position, rotation or scale, just update those aspects of the matrix

void CGameObject::SetPosition(CVector3 position, int node) { mWorldMatrices[node].SetRow(3, position); }

void CGameObject::SetRotation(CVector3 rotation, int node)
{
	// To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
	mWorldMatrices[node] = MatrixScaling(Scale(node)) *
		MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
		MatrixTranslation(Position(node));
}

// Two ways to set scale: x,y,z separately, or all to the same value
// To set scale without affecting rotation, normalise each row, then multiply it by the scale value.

void CGameObject::SetScale(CVector3 scale, int node)
{
	mWorldMatrices[node].SetRow(0, Normalise(mWorldMatrices[node].GetRow(0)) * scale.x);
	mWorldMatrices[node].SetRow(1, Normalise(mWorldMatrices[node].GetRow(1)) * scale.y);
	mWorldMatrices[node].SetRow(2, Normalise(mWorldMatrices[node].GetRow(2)) * scale.z);
}

void CGameObject::SetScale(float scale) { SetScale({ scale, scale, scale }); }

void CGameObject::SetWorldMatrix(CMatrix4x4 matrix, int node) { mWorldMatrices[node] = matrix; }
