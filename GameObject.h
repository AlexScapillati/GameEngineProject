//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.

#pragma once

#include "CVector3.h"
#include "CMatrix4x4.h"
#include "Input.h"
#include <string>
#include <vector>
#include "Mesh.h"
#include <stdexcept>
#include "Material.h"

class CMesh;

class CGameObject
{
public:
	//-------------------------------------
	// Construction / Usage
	//-------------------------------------

	CGameObject(std::string mesh,std::string name, std::string& diffuseMap, std::string& vertexShader,
	            std::string& pixelShader, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1);
	
	virtual void Render(bool basicGeometry = false);

	void Control(int node, float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
		KeyCode turnCW, KeyCode turnCCW, KeyCode moveForward, KeyCode moveBackward);

	//-------------------------------------
	// Data access
	//-------------------------------------

	//********************************
	// All functions now accept a "node" parameter which specifies which node in the hierarchy to use. Defaults to 0, the root.
	// The hierarchy is stored in depth-first order

	// Getters - model only stores matrices. Position, rotation and scale are extracted if requested.
	CVector3 Position(int node = 0);         // Position is on bottom row of matrix
	CVector3 Rotation(int node = 0);  // Getting angles from a matrix is complex - see .cpp file
	CVector3 Scale(int node = 0); // Scale is length of rows 0-2 in matrix
	CMatrix4x4 WorldMatrix(int node = 0);

	//get the directs access to the position of the model
	float* DirectPosition();

	CMesh* GetMesh() const;

	auto GetName() { return mName; }

	auto Enabled() { return &mEnabled; }

	auto GetTextureSRV() { return mMaterial->GetTexture(); }
	
	auto GetTexture() { return mMaterial->GetTextureSRV();}

	// Setters - model only stores matricies , so if user sets position, rotation or scale, just update those aspects of the matrix
	void SetPosition(CVector3 position, int node = 0);

	virtual void SetRotation(CVector3 rotation, int node = 0);

	// Two ways to set scale: x,y,z separately, or all to the same value
	// To set scale without affecting rotation, normalize each row, then multiply it by the scale value.
	void SetScale(CVector3 scale, int node = 0);

	void SetScale(float scale);

	void SetWorldMatrix(CMatrix4x4 matrix, int node = 0);

	bool Update(float updateTime);

	virtual ~CGameObject();

	//-------------------------------------
	// Private data / members
	//-------------------------------------


protected:

	
	//the material
	CMaterial* mMaterial;

	//the meshes that a model has (all the LODS that a model has)
	std::vector<std::string> mMeshFiles;

	CMesh* mMesh;
	
	std::string mName;

	bool mEnabled;
	

	// World matrices for the model
	// Now that meshes have multiple parts, we need multiple matrices. The root matrix (the first one) is the world matrix
	// for the entire model. The remaining matrices are relative to their parent part. The hierarchy is defined in the mesh (nodes)
	std::vector<CMatrix4x4> mWorldMatrices;

};

