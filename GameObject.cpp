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


CGameObject::CGameObject(std::string mesh, std::string name, std::string& diffuseMap, std::string&
	vertexShader, std::string& pixelShader, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/)
{

	mEnabled = true;

	if (mesh.empty()) throw std::exception("Error Loading Object");

	mName = std::move(name);

	//import material
	try
	{
		mMaterial = new CMaterial(diffuseMap, vertexShader, pixelShader);
	}
	catch (std::exception e)
	{
		throw std::runtime_error(e.what());
	}

	if (mMaterial->IsPbr())
	{
		//if this model has a normal map
		if (mMaterial->HasNormals())
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

	//render the material

	mMaterial->RenderMaterial(basicGeometry);

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
