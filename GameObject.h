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


class CMesh;

class CGameObject
{
public:
	//-------------------------------------
	// Construction / Usage
	//-------------------------------------

	CGameObject(std::string mesh,std::string name, const std::string& diffuseMap, std::string& vertexShader,
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

	CMesh* GetMesh() const;

	// Setters - model only stores matricies , so if user sets position, rotation or scale, just update those aspects of the matrix
	void SetPosition(CVector3 position, int node = 0);

	void SetRotation(CVector3 rotation, int node = 0);

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

	//for regular models
	//can actually be replaced by the albedo
	
	ID3D11Resource*				mDiffuseSpecularMap;
	ID3D11ShaderResourceView*	mDiffuseSpecularMapSRV;

	ID3D11VertexShader*		mVertexShader;
	ID3D11GeometryShader*   mGeometryShader; //WIP
	ID3D11PixelShader*		mPixelShader;

	//All the pbr related maps that a model can have
	struct sPbrMaps
	{
		ID3D11Resource*				Albedo;
		ID3D11ShaderResourceView*	AlbedoSRV;
		ID3D11Resource*				AO;
		ID3D11ShaderResourceView*	AoSRV;
		ID3D11Resource*				Displacement;
		ID3D11ShaderResourceView*	DisplacementSRV;
		ID3D11Resource*				Normal;
		ID3D11ShaderResourceView*	NormalSRV;
		ID3D11Resource*				Roughness;
		ID3D11ShaderResourceView*	RoughnessSRV;
	} mPbrMaps;

	//the meshes that a model has (all the LODS that a model has)
	std::vector<std::string> mMeshFiles;

	CMesh* mMesh;
	
	std::string mName;

	// World matrices for the model
	// Now that meshes have multiple parts, we need multiple matrices. The root matrix (the first one) is the world matrix
	// for the entire model. The remaining matrices are relative to their parent part. The hierarchy is defined in the mesh (nodes)
	std::vector<CMatrix4x4> mWorldMatrices;

};

