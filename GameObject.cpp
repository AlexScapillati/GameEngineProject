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
#include <utility>
#include "ColourRGBA.h"
#include "GameObjectManager.h"

CGameObject::CGameObject(const CGameObject& obj)
{
	mEnabled = true;
	mMaterial = std::make_unique<CMaterial>(*obj.mMaterial);
	mMeshFiles = obj.mMeshFiles;
	mMesh = std::make_unique<CMesh>(*obj.mMesh);
	mName = "new" + obj.mName;

	mAmbientMap.Init();

	// Set default matrices from mesh
	mWorldMatrices.resize(mMesh->NumberNodes());
	for (auto i = 0; i < mWorldMatrices.size(); ++i)
		mWorldMatrices[i] = mMesh->GetNodeDefaultMatrix(i);
}

CGameObject::CGameObject(std::string mesh, std::string name, std::string& diffuseMap, std::string&
	vertexShader, std::string& pixelShader, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/)
{
	mAmbientMap.enabled = false;
	mAmbientMap.size = 256;
	mAmbientMap.Init();

	mEnabled = true;

	if (mesh.empty()) throw std::exception("Error Loading Object");

	mName = std::move(name);

	//import material
	try
	{
		mMaterial = std::make_unique<CMaterial>(diffuseMap, vertexShader, pixelShader);
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
		mMesh = std::make_unique<CMesh>(mesh);

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

CGameObject::CGameObject(std::string id, std::string name, std::string vs, std::string ps, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/)
{
	//initialize member variables
	mName = std::move(name);

	mAmbientMap.Init();

	mEnabled = true;

	//search for files with the same id
	std::vector<std::string>files;

	GetFilesWithID(gMediaFolder, files, id);

	//create the material
	mMaterial = std::make_unique<CMaterial>(files, vs, ps);

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
		throw std::runtime_error(name);
	}

	try
	{
		//load the most detailed mesh with tangents required
		mMesh = std::make_unique<CMesh>(mMeshFiles.front(), mMaterial->HasNormals());

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

// The render function simply passes this model's matrices over to Mesh:Render.
// All other per-frame constants must have been set already along with shaders, textures, samplers, states etc.
void CGameObject::Render(bool basicGeometry)
{
	//if the model is not enable do not render it
	if (!mEnabled) return;

	//render the material
	mMaterial->RenderMaterial(basicGeometry);

	//render the ambient cube map
	if (mAmbientMap.enabled)
	{
		RenderToAmbientMap();

		//send the cubemap to the shader (slot 5)
		gD3DContext->PSSetShaderResources(5, 1, &mAmbientMap.mapSRV);
	}

	//TODO render the the correct mesh according to the camera distance
	mMesh->Render(mWorldMatrices);

	ID3D11ShaderResourceView* nullView = nullptr;
	gD3DContext->PSSetShaderResources(5, 1, &nullView);
}

bool CGameObject::Update(float updateTime)
{
	return true; //TODO WIP
}

void CGameObject::RenderToAmbientMap()
{
	//create the viewport 
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(mAmbientMap.size);
	vp.Height = static_cast<FLOAT>(mAmbientMap.size);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gD3DContext->RSSetViewports(1, &vp);


	float mSides[6][3] = {
			{ 1.0f,	 0.0f,	 0.0f},
			{-1.0f,	 0.0f,	 0.0f},
			{ 0.0f,	 1.0f,	 0.0f},
			{ 0.0f, -1.0f,	 0.0f},
			{ 0.0f,	 0.0f,	 1.0f},
			{ 0.0f,	 0.0f,  -1.0f}
	};

	auto matrix = WorldMatrix();

	//for all six faces of the cubemap
	for (int i = 0; i < 6; ++i)
	{
		//change rotation
		SetRotation(CVector3(mSides[i]) * PI);

		float c[] = { 0.0f,0.0f,0.0f ,0.0f };

		gD3DContext->ClearRenderTargetView(mAmbientMap.RTV[i], c);
		gD3DContext->ClearDepthStencilView(mAmbientMap.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		gD3DContext->OMSetRenderTargets(6, mAmbientMap.RTV, mAmbientMap.depthStencilView);

		gPerFrameConstants.viewMatrix = InverseAffine(WorldMatrix());
		gPerFrameConstants.projectionMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.0f));
		gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;

		UpdateFrameConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

		gD3DContext->VSSetConstantBuffers(1, 1, &gPerFrameConstantBuffer);

		//render just the objects that can cast shadows
		for (auto it : GOM->mObjects)
		{
			if (it != this)
				//render with all the fancy shaders
				it->Render();
		}
	}

	//restore prev matrix
	SetWorldMatrix(matrix);

	//generate mipMaps for the cube map
	gD3DContext->GenerateMips(mAmbientMap.mapSRV);

}

void CGameObject::sAmbientMap::Init()
{
	//initialize ambient map variables
	size = 256;
	enabled = false;

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
	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &map)))
	{
		throw std::runtime_error("Error creating cube texture");
	}

	//create render target views
	D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
	viewDesc.Format = textureDesc.Format;
	viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.ArraySize = 1;
	viewDesc.Texture2DArray.MipSlice = 0;

	//create 6 of them
	for (int i = 0; i < 6; ++i)
	{
		viewDesc.Texture2DArray.FirstArraySlice = i;

		if (FAILED(gD3DDevice->CreateRenderTargetView(map, &viewDesc, &RTV[i])))
		{
			throw std::runtime_error("Error creating render target view");
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	if (FAILED(gD3DDevice->CreateShaderResourceView(map, &srvDesc, &mapSRV)))
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
	dsDesc.ArraySize = 1; //6 faces
	dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	//create it
	if (FAILED(gD3DDevice->CreateTexture2D(&dsDesc, NULL, &depthStencilMap)))
	{
		throw std::runtime_error("Error creating depth stencil");
	}

	//create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(gD3DDevice->CreateDepthStencilView(depthStencilMap, &dsvDesc, &depthStencilView)))
	{
		throw std::runtime_error("Error creating depth stencil view ");
	}

	//release the depth stencil texture since we are done with it
	depthStencilMap->Release();
}

CGameObject::~CGameObject()
{
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

CVector3 CGameObject::Scale(int node) {
	return { Length(mWorldMatrices[node].GetRow(0)),
		Length(mWorldMatrices[node].GetRow(1)),
		Length(mWorldMatrices[node].GetRow(2)) };
}

// Scale is length of rows 0-2 in matrix

CMatrix4x4& CGameObject::WorldMatrix(int node) { return mWorldMatrices[node]; }

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


CMesh* CGameObject::GetMesh() const { return mMesh.get(); }

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

void CGameObject::SetSize(UINT s)
{
	mAmbientMap.size = s;

	//TODO remake the texture
}

bool* CGameObject::AmbientMapEnabled()
{
	return &mAmbientMap.enabled;
}

void CGameObject::sAmbientMap::Release()
{
	depthStencilMap->Release();
	depthStencilView->Release();
	map->Release();
	mapSRV->Release();
	for (auto it : RTV)
	{
		it->Release();
	}
	enabled = false;
}
