//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.

#include <codecvt>
#include <locale>
#include <utility>

#include "GameObject.h"
#include "..\Material.h"
#include "..\Mesh.h"
#include "../GraphicsHelpers.h"
#include "..\DX11Engine.h"
#include "..\Scene.h"



//copy constructor
CDX11GameObject::CDX11GameObject(CDX11GameObject& obj)
{
	mEngine = obj.mEngine;
	mEnabled = true;
	mMaterial = new CMaterial(*obj.mMaterial);
	mMeshFiles = obj.mMeshFiles;
	mMesh = new CDX11Mesh(mEngine, obj.mMesh->MeshFileName(), mMaterial->HasNormals());
	mName = "new" + obj.mName;

	mLODs = obj.mLODs;
	mCurrentLOD = obj.mCurrentLOD;
	mCurrentVar = obj.mCurrentVar;

	mParallaxDepth = obj.ParallaxDepth();
	mRoughness = obj.Roughness();
	mMetalness = obj.mMetalness;

	//initialize ambient map variables
	mAmbientMap.size = obj.AmbientMap()->size;
	mAmbientMap.enabled = obj.AmbientMap()->enabled;
	mAmbientMap.Init(mEngine);

	// Set default matrices from mesh
	mWorldMatrices.resize(mMesh->NumberNodes());
	for (auto i = 0; i < mWorldMatrices.size(); ++i)
		mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);

	SetPosition(obj.Position());
	SetRotation(obj.Rotation());
	SetScale(obj.Scale());
}

CDX11GameObject::CDX11GameObject(CDX11Engine* engine, std::string mesh, std::string name, std::string& diffuseMap, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/)
{
	mEngine = engine;
	mAmbientMap.enabled = false;
	mAmbientMap.size = 4;
	mAmbientMap.Init(mEngine);

	mParallaxDepth = 0.f;
	mRoughness = 0.5f;
	mMetalness = 0.0f;

	mEnabled = true;

	if (mesh.empty()) throw std::exception("Error Loading Object");

	mName = std::move(name);

	// Import material
	try
	{
		std::vector<std::string> maps = { diffuseMap };
		mMaterial = new CMaterial(maps,mEngine);
	}
	catch (std::exception e)
	{
		throw std::runtime_error(e.what());
	}

	//default model
	//not PBR
	//that could be light models or cube maps
	try
	{
		mMesh = new CDX11Mesh(mEngine,mesh, mMaterial->HasNormals());
		mMeshFiles.push_back(mesh);

		// Set default matrices from mesh
		mWorldMatrices.resize(mMesh->NumberNodes());
		for (auto i = 0; i < mWorldMatrices.size(); ++i)
			mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
	}
	catch (std::exception& e)
	{
		throw std::runtime_error(e.what());
	}

	//geometry loaded, set its position...

	SetPosition(position);
	SetRotation(rotation);
	SetScale(scale);
}

void GetFilesWithID(std::string& dirPath, std::vector<std::string>& fileNames, std::string& id)
{
	//iterate through the directory
	std::filesystem::recursive_directory_iterator iter(dirPath);

	std::filesystem::recursive_directory_iterator end;

	while (iter != end)
	{
		if (!is_directory(iter->path()))
		{
			auto filename = iter->path().filename().string();

			if (filename.find(id) != std::string::npos)
			{
				fileNames.push_back(iter->path().filename().string());
				iter.disable_recursion_pending();
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



void CDX11GameObject::GetFilesInFolder(std::string& dirPath, std::vector<std::string>& fileNames)
{
	//iterate through the directory
	std::filesystem::recursive_directory_iterator iter(dirPath);

	dirPath.replace(0, mEngine->GetMediaFolder().size(), "");

	if (dirPath[dirPath.size() - 1] != '/') dirPath.push_back('/');

	std::filesystem::recursive_directory_iterator end;

	while (iter != end)
	{
		if (!is_directory(iter->path()))
		{
			fileNames.push_back(dirPath + iter->path().filename().string());
			iter.disable_recursion_pending();
		}
		std::error_code ec;
		iter.increment(ec);
		if (ec)
		{
			throw std::runtime_error("Error accessing " + ec.message());
		}
	}
}

CDX11GameObject::CDX11GameObject(CDX11Engine* engine, std::string dirPath, std::string name, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/)
{

	mEngine = engine;

	//initialize member variables
	mName = std::move(name);

	//initialize ambient map variables
	mAmbientMap.size = 4;
	mAmbientMap.enabled = false;
	mAmbientMap.Init(mEngine);

	mEnabled = true;

	mParallaxDepth = 0.f;
	mRoughness = 0.5f;
	mMetalness = 0.0f;

	//search for files with the same id
	std::vector<std::string>files;

	std::string id = dirPath;

	auto folder = engine->GetMediaFolder() + dirPath;

	//	If the id has a dot, this means it has an extention, so we are dealing with a file
	if (id.find_last_of('.') == std::string::npos)
	{
		// Get every file in that folder
		// If the id did not have a dot means that we are dealing with a folder
		GetFilesInFolder(folder, files);
	}
	else
	{
		// Get the position of the last underscore
		auto nPos = id.find_last_of('_');

		//Get folder
		auto slashPos = id.find_last_of('/');

		if (slashPos != std::string::npos)
		{
			// Get the id
			auto subFolder = id.substr(0, slashPos);


			folder = engine->GetMediaFolder() + subFolder + '/';

			//get every file that is beginning with that id
			GetFilesInFolder(folder, files);
		}
		else
		{
			// Get the ID
			id = id.substr(0, id.find_first_of('_'));

			GetFilesWithID(engine->GetMediaFolder(), files, id);
		}
	}

	//create the material
	mMaterial = new CMaterial(files,mEngine);

	//find meshes trough the files
	for (auto st : files)
	{
		//set the meshes in the vector
		if (st.find(".fbx") != std::string::npos)
		{
			mMeshFiles.push_back(st);
		}
		else if (st.find(".x") != std::string::npos)
		{
			mMeshFiles.push_back(st);
		}
	}

	if (mMeshFiles.empty())
	{
		throw std::runtime_error("No mesh found in " + name);
	}

	// If the meshes vector has more than one fileName
	if (mMeshFiles.size() > 1)
	{
		// Extract the lods and variations
		int counter = 0;
		int nVariations = 0;
		std::vector<std::string> vec;
		for (auto mesh : mMeshFiles)
		{
			// Prepare file to find
			// It changes depending on the counter
			std::string currFind = "LOD";
			currFind += (counter + '0');

			// If found
			if (mesh.find(currFind) != std::string::npos)
			{
				// Push it in the variations vector
				vec.push_back(mesh);
			}
			else
			{
				// If not found we finished the variations for this LOD
				// So push the variations vector in the LODs vector and clear the variations vector
				mLODs.push_back(move(vec));

				// Push the current variation iin the variation vec
				vec.push_back(mesh);

				// Increment the LOD counter
				counter++;
			}
		}
	}
	else
	{
		// If the meshes vector has only one fileName
		// Push it on the lod
		mLODs.push_back(mMeshFiles);
	}

	try
	{
		//load the most detailed mesh with tangents required if the model has normals
		mMesh = new CDX11Mesh(mEngine,mMeshFiles.front(), mMaterial->HasNormals());

		// Set default matrices from mesh
		mWorldMatrices.resize(mMesh->NumberNodes());
		for (auto i = 0; i < mWorldMatrices.size(); ++i)
			mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
	}
	catch (std::exception& e)
	{
		throw std::runtime_error(e.what());
	}

	// geometry loaded, set its position...
	SetPosition(position);
	SetRotation(rotation);
	SetScale(scale);
}

// The render function simply passes this model's matrices over to Mesh:Render.
// All other per-frame constants must have been set already along with shaders, textures, samplers, states etc.
void CDX11GameObject::Render(bool basicGeometry)
{
	//if the model is not enable do not render it
	if (!mEnabled) return;

	// Set the parallax depth to the constant buffer
	gPerModelConstants.parallaxDepth = mParallaxDepth;

	// Set the material roughness to the constant buffer
	gPerModelConstants.roughness = mRoughness;
	gPerModelConstants.metalness = mMetalness;

	if (!basicGeometry)
	{
		if (mAmbientMap.enabled)
		{
			mEngine->GetContext()->PSSetShaderResources(6, 1, &mAmbientMap.mapSRV);
		}
		/*else if (dynamic_cast<CSky*>(GOM->GetSky())->HasCubeMap());
		{
			auto environmentMap = GOM->GetSky()->TextureSRV();
			gD3DContext->PSSetShaderResources(6, 1, &environmentMap);
		}*/
	}

	// Render the material
	mMaterial->RenderMaterial(basicGeometry);

	// Render the mesh
	mMesh->Render(mWorldMatrices);

	// Unbind the ambient map from the shader
	ID3D11ShaderResourceView* nullView = nullptr;
	mEngine->GetContext()->PSSetShaderResources(6, 1, &nullView);
}

bool CDX11GameObject::Update(float updateTime)
{
	return true; //TODO WIP
}

void CDX11GameObject::RenderToAmbientMap()
{
	// if the ambient map is disabled, nothing to do here
	if (!mAmbientMap.enabled) return;

	// Store current RS state
	ID3D11RasterizerState* prevRS = nullptr;
	mEngine->GetContext()->RSGetState(&prevRS);

	// Set the cullback state
	mEngine->GetContext()->RSSetState(mEngine->mCullBackState.Get());

	// Store current RTV(render target) and DSV(depth stencil)
	ID3D11RenderTargetView* prevRTV = nullptr;
	ID3D11DepthStencilView* prevDSV = nullptr;

	mEngine->GetContext()->OMGetRenderTargets(1, &prevRTV, &prevDSV);

	//create the viewport
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(mAmbientMap.size);
	vp.Height = static_cast<FLOAT>(mAmbientMap.size);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mEngine->GetContext()->RSSetViewports(1, &vp);

	float mSides[6][3] = {          // Starting from facing down the +ve Z direction, left handed rotations
			{ 0.0f,	 0.5f,	0.0f},  // +ve X direction (values multiplied by PI)
			{ 0.0f, -0.5f,	0.0f},  // -ve X direction
			{-0.5f,	 0.0f,	0.0f},  // +ve Y direction
			{ 0.5f,	 0.0f,	0.0f},  // -ve Y direction
			{ 0.0f,	 0.0f,	0.0f},  // +ve Z direction
			{ 0.0f,	 1.0f,  0.0f}   // -ve Z direction
	};

	const auto& originalRotation = Rotation();


	// for all six faces of the cubemap
	for (int i = 0; i < 6; ++i)
	{
		// change rotation
		SetRotation(CVector3(mSides[i]) * PI);

		mEngine->GetContext()->ClearDepthStencilView(mAmbientMap.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		mEngine->GetContext()->OMSetRenderTargets(1, &mAmbientMap.RTV[i], mAmbientMap.depthStencilView);

		// Update Frame buffer
		gPerFrameConstants.viewMatrix = InverseAffine(WorldMatrix());
		gPerFrameConstants.projectionMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.0f));
		gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;

		// Update Constant buffer
		mEngine->UpdateFrameConstantBuffer(gPerFrameConstantBuffer.Get(), gPerFrameConstants);

		// Set it to the GPU
		mEngine->GetContext()->PSSetConstantBuffers(1, 1, gPerFrameConstantBuffer.GetAddressOf());
		mEngine->GetContext()->VSSetConstantBuffers(1, 1, gPerFrameConstantBuffer.GetAddressOf());

		//render just the objects that can cast shadows
		for (auto& it : mEngine->GetScene()->GetObjectManager()->mObjects)
		{
			// Do not render this object
			if (it != this)
				// Render all the objects 
				// Performance improvements: Could render only the closest objects
				// Could render only the face that the user is looking at (kinda like cull back state)
				it->Render();
		}
	}

	//restore render target, otherwise the ambient map will not be sent to the shader because it is still bound as a render target
	mEngine->GetContext()->OMSetRenderTargets(1, &prevRTV, prevDSV);

	if (prevRTV) prevRTV->Release();
	if (prevDSV) prevDSV->Release();

	// Restore previous RS state
	mEngine->GetContext()->RSSetState(prevRS);

	// Release that
	if (prevRS) prevRS->Release();

	//restore original rotation
	SetRotation(originalRotation);

	//generate mipMaps for the cube map
	mEngine->GetContext()->GenerateMips(mAmbientMap.mapSRV);
}

std::vector<std::string>& CDX11GameObject::GetMeshes()
{
	return mMeshFiles;
}

void CDX11GameObject::LoadNewMesh(std::string newMesh)
{
	try
	{
		auto newCMesh = new CDX11Mesh(mEngine,newMesh, mMaterial->HasNormals());

		delete mMesh;

		mMesh = newCMesh;

		auto prevPos = Position();
		auto prevScale = Scale();
		auto prevRotation = Rotation();

		// Recalculate matrix based on mesh
		mWorldMatrices.resize(mMesh->NumberNodes());
		for (auto i = 0; i < mWorldMatrices.size(); ++i)
			mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);

		SetPosition(prevPos);
		SetScale(prevScale);
		SetRotation(prevRotation);

	}
	catch (std::exception& e)
	{
		throw std::exception(e.what());
	}


}

// Set the given mesh variation
// It will check if the variation is valid
// Not performance friendly, it will delete the current mesh and load a new one

void CDX11GameObject::SetVariation(int variation)
{
	if (variation >= 0 && variation < mLODs[mCurrentLOD].size())
		LoadNewMesh(mLODs[mCurrentLOD][variation]);
}

void CDX11GameObject::sAmbientMap::Init(CDX11Engine* engine)
{
	mEngine = engine;

	//http://richardssoftware.net/Home/Post/26

	//initialize the texture map cube
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = size;
	textureDesc.Height = size;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 6; //6 faces
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; //use it to passit to the shader and use it as a render target
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	//create it
	auto hr = mEngine->GetDevice()->CreateTexture2D(&textureDesc, NULL, &map);
	if (FAILED(hr))
	{
		throw std::runtime_error("Error creating cube texture");
	}

	//create render target views
	D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
	viewDesc.Format = textureDesc.Format;
	viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = 1;
	viewDesc.Texture2DArray.MipSlice = 0;

	//create 6 of them
	for (int i = 0; i < 6; ++i)
	{
		viewDesc.Texture2DArray.FirstArraySlice = i;
		hr = mEngine->GetDevice()->CreateRenderTargetView(map, &viewDesc, &RTV[i]);
		if (FAILED(hr))
		{
			throw std::runtime_error("Error creating render target view");
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	hr = mEngine->GetDevice()->CreateShaderResourceView(map, &srvDesc, &mapSRV);
	if (FAILED(hr))
	{
		throw std::runtime_error("Error creating cube map SRV");
	}

	//now can release the texture
	map->Release();

	//create depth stencil
	D3D11_TEXTURE2D_DESC dsDesc = {};
	dsDesc.Width = size;
	dsDesc.Height = size;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	//create it
	if (FAILED(mEngine->GetDevice()->CreateTexture2D(&dsDesc, NULL, &depthStencilMap)))
	{
		throw std::runtime_error("Error creating depth stencil");
	}

	//create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(mEngine->GetDevice()->CreateDepthStencilView(depthStencilMap, &dsvDesc, &depthStencilView)))
	{
		throw std::runtime_error("Error creating depth stencil view ");
	}

	//release the depth stencil texture since we are done with it
	depthStencilMap->Release();
}

CDX11GameObject::~CDX11GameObject()
{
	CDX11GameObject::Release();
}


void CDX11GameObject::Release() 
{
	if (mMesh) delete mMesh;
	if (mMaterial) delete mMaterial;

	mAmbientMap.Release();
}

// Control a given node in the model using keys provided. Amount of motion performed depends on frame time
void CDX11GameObject::Control(int node, float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
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

CVector3 CDX11GameObject::Position(int node) { return mWorldMatrices[node].GetRow(3); }

// Position is on bottom row of matrix

CVector3 CDX11GameObject::Rotation(int node) { return mWorldMatrices[node].GetEulerAngles(); }

// Getting angles from a matrix is complex - see .cpp file

CVector3 CDX11GameObject::Scale(int node)
{
	return {
		Length(mWorldMatrices[node].GetRow(0)),
		Length(mWorldMatrices[node].GetRow(1)),
		Length(mWorldMatrices[node].GetRow(2))
	};
}

// Scale is length of rows 0-2 in matrix

CMatrix4x4& CDX11GameObject::WorldMatrix(int node) { return mWorldMatrices[node]; }

float* CDX11GameObject::DirectPosition()
{
	return &mWorldMatrices[0].e30;
}

CDX11Mesh* CDX11GameObject::Mesh() const { return mMesh; }

std::string CDX11GameObject::Name() const
{ return mName; }

void CDX11GameObject::SetName(std::string n)
{ mName = n; }

float& CDX11GameObject::ParallaxDepth()
{ return mParallaxDepth; }

void CDX11GameObject::SetParallaxDepth(float p)
{ mParallaxDepth = p; }

bool* CDX11GameObject::Enabled()
{ return &mEnabled; }

std::string CDX11GameObject::TextrueFileName() const
{ return mMaterial->TextureFileName(); }

CMaterial* CDX11GameObject::Material()
{ return mMaterial; }

float& CDX11GameObject::Roughness()
{ return mRoughness; }

float& CDX11GameObject::Metalness()
{ return mMetalness; }

void CDX11GameObject::SetRoughness(float r)
{ mRoughness = r; }

std::vector<std::string>& CDX11GameObject::GetVariations()
{ return mLODs[mCurrentLOD]; }

int CDX11GameObject::CurrentLOD() const
{ return mCurrentLOD; }

std::vector<std::vector<std::string>> CDX11GameObject::LODs() const
{ return mLODs; }

int CDX11GameObject::CurrentVariation() const
{ return mCurrentVar; }

void CDX11GameObject::SetLOD(int i)
{ 
	if (i > 0 && i < mLODs.size()) 
		LoadNewMesh(mLODs[i][mCurrentVar]); 
}

CDX11GameObject::sAmbientMap* CDX11GameObject::AmbientMap()
{ return &mAmbientMap; }

ID3D11ShaderResourceView* CDX11GameObject::AmbientMapSRV() const
{ return mAmbientMap.mapSRV; }

UINT CDX11GameObject::sAmbientMap::Size() const
{ return size; }

std::string CDX11GameObject::MeshFileNames()
{ return mMeshFiles.front(); }

// Setters - model only stores matricies , so if user sets position, rotation or scale, just update those aspects of the matrix

ID3D11ShaderResourceView* CDX11GameObject::TextureSRV() const
{ return mMaterial->TextureSRV(); }

void CDX11GameObject::SetPosition(CVector3 position, int node) { mWorldMatrices[node].SetRow(3, position); }

void CDX11GameObject::SetRotation(CVector3 rotation, int node)
{
	// To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
	mWorldMatrices[node] = MatrixScaling(Scale(node)) *
		MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
		MatrixTranslation(Position(node));
}

// Two ways to set scale: x,y,z separately, or all to the same value
// To set scale without affecting rotation, normalise each row, then multiply it by the scale value.

void CDX11GameObject::SetScale(CVector3 scale, int node)
{
	mWorldMatrices[node].SetRow(0, Normalise(mWorldMatrices[node].GetRow(0)) * scale.x);
	mWorldMatrices[node].SetRow(1, Normalise(mWorldMatrices[node].GetRow(1)) * scale.y);
	mWorldMatrices[node].SetRow(2, Normalise(mWorldMatrices[node].GetRow(2)) * scale.z);
}

void CDX11GameObject::SetScale(float scale) { SetScale({ scale, scale, scale }); }

void CDX11GameObject::SetWorldMatrix(CMatrix4x4 matrix, int node) { mWorldMatrices[node] = matrix; }


bool& CDX11GameObject::AmbientMapEnabled()
{
	return mAmbientMap.enabled;
}

void CDX11GameObject::sAmbientMap::SetSize(UINT s)
{
	// Release all the maps
	Release();

	// Set the size
	size = s;

	// Re initialize all the maps to the new size
	Init(mEngine);

}

void CDX11GameObject::sAmbientMap::Release()
{
	if (depthStencilView) depthStencilView->Release();
	if (mapSRV) mapSRV->Release();
	for (auto& it : RTV)
	{
		if (it) it->Release();
	}
}