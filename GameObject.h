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
class CGameObjectManager;


class CGameObject
{
public:

	//-------------------------------------
	// Construction / Usage
	//-------------------------------------

	// Copy constructor
	// Deep copy
	CGameObject(CGameObject&);

	// Simple object contructor
	// A mesh and a diffuse map are compulsory to render a model
	CGameObject(std::string mesh, std::string name, std::string& diffuseMap, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1);

	// "Smart" Constructor
	// Given an ID (That could be a file or a directory) this constructor will import all the files in that folder or the files that will contain that id
	// A GameObject to be rendered needs at least a mesh and a texture
	// Formats:
	// Folders: NAME_ID_TYPE
	// Meshes:	NAME_LOD_VARIATION.EXTENTION
	// Textures: ID_RESOLUTION_TYPE.EXTENTION
	CGameObject(std::string id, std::string name, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1);

	// Render the object
	// Accepts a boolean value for rendering just the mesh 
	// Used in depth passes where the material does not need to be rendered
	virtual void Render(bool basicGeometry = false);

	void Control(int node, float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
		KeyCode turnCW, KeyCode turnCCW, KeyCode moveForward, KeyCode moveBackward);

	// Virtual Destructor
	// Needed for every child classes
	virtual ~CGameObject();

	virtual void Release();

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
	CMatrix4x4& WorldMatrix(int node = 0);

	float* DirectPosition();

	CMesh* Mesh() const;

	auto MeshFileNames() { return mMeshFiles.front(); }

	auto Name() { return mName; }

	auto SetName(std::string n) { mName = n; }

	auto& ParallaxDepth() { return mParallaxDepth; }

	auto SetParallaxDepth(float p) { mParallaxDepth = p; }

	auto Enabled() { return &mEnabled; }

	auto TextureSRV() { return mMaterial->TextureSRV(); }

	auto TextrueFileName() { return mMaterial->TextureFileName(); }

	auto& Material() { return mMaterial; }

	auto& Roughness() { return mRoughness; }

	auto SetRoughness(float r) { mRoughness = r; }

	// Setters - model only stores matricies , so if user sets position, rotation or scale, just update those aspects of the matrix
	void SetPosition(CVector3 position, int node = 0);

	virtual void SetRotation(CVector3 rotation, int node = 0);

	// Two ways to set scale: x,y,z separately, or all to the same value
	// To set scale without affecting rotation, normalize each row, then multiply it by the scale value.
	void SetScale(CVector3 scale, int node = 0);

	// Similar function to the one above, Thiss will set the same scale to all three axis
	void SetScale(float scale);

	// This function will set the model world matrix
	void SetWorldMatrix(CMatrix4x4 matrix, int node = 0);

	// Set the value of mChangedPos
	// Static since we are using a static variable
	void SetChangedPos(bool b) { mChangedPos = b; }

	// WIP, This will handle all the scripts and update the model's behaviour (similar to unity)
	bool Update(float updateTime);

	// Return the vector holding the meshes filenames
	std::vector<std::string>& GetMeshes();

	// Delete the current mesh and load the given one. It will not delete the current if the filename is wrong
	void LoadNewMesh(std::string newMesh);

	//-------------------------------------
	// Level of Detail and Mesh Variations
	//-------------------------------------

	// The functions below are for the the management of the various LODs and mesh variations that a model can have
	// It assumes that the number of variations for each model are the same E.g. 3 LODs with 3 variations each LOD (9 meshes)
	// For performance reason is better to load one mesh initilly than load every lod and variations at startup.
	// So when the user wants it it can change the lod or variation in runtime loading the corrected mesh.
	// This however will freeze the current frame because assimp will need to load the mesh.

	// This function will return all the mesh variations filenames for the current LOD (Level of Detail)
	std::vector<std::string>& GetVariations() { return mLODs[mCurrentLOD]; }

	// Set the given mesh variation
	// It will check if the variation is valid
	// Not performance friendly, it will delete the current mesh and load a new one
	void SetVariation(int variation);

	// Return the current LOD
	int CurrentLOD() { return mCurrentLOD; }

	auto LODs() { return mLODs; }

	int CurrentVariation() { return mCurrentVar; }

	void SetLOD(int i) 
	{ 
		if (i > 0 && i < mLODs.size()) 
			LoadNewMesh(mLODs[i][mCurrentVar]); 
	}

	//-------------------------------------
	// Ambient Map (Global Illumination)
	//-------------------------------------

	// Returns if the ambient map is enabled
	bool& AmbientMapEnabled();

	// Returns the actual ambient map struct
	auto AmbientMap() { return &mAmbientMap; }

	// Returns the ambient map shader resource view (for passing it to the shaders)
	auto AmbientMapSRV() { return mAmbientMap.mapSRV; }

	// Render the whole scene from the model perspective. Very expensive, needs optimization
	void RenderToAmbientMap();

	//-------------------------------------
	// Private data / members
	//-------------------------------------

	// This variable decides whether to render a ambient map or not.
	// Since the rendering of an ambient map could be expensive
	// Static is to ensure that this variable is linked to every instance of this class.
	// This because if an object changed its position, this instance will need to re-render the ambient map
	bool mChangedPos;

protected:


	// The material
	// It will hold all the textures and send them to the shader with RenderMaterial()
	CMaterial* mMaterial;

	//the meshes that a model has (all the LODS that a model has)
	std::vector<std::string> mMeshFiles;

	// All the lods that a mesh has, every lod will have multiple variations if any
	std::vector<std::vector<std::string>> mLODs;

	// Store the current LOD and mesh variation rendered
	int mCurrentLOD = 0;
	int mCurrentVar = 0;

	// The acutual mesh class
	CMesh* mMesh;

	// Each model has a parallax depth value 
	// For the models that have a displacement and a normal map this will modify the bumpyness of those textures
	float mParallaxDepth;

	// Each model has a roughness value. This because not every model will have a roguhness map. So we can change its roughness manually.
	// If a model has a roughness map. This will not make any changes.
	float mRoughness;

	std::string mName;

	// This value controls the model visibility. If it is false the model will not be rendered (not cast/make shadows)
	bool mEnabled;

	// The Ambient Map
	// Its purpose is to render the scene surrounding the object to a cubemap
	// Then it will send the cubemap to the shader and display reflexes on the object
	struct sAmbientMap
	{
		// Initialize the textures
		void Init();

		bool enabled;
		ID3D11Texture2D* map;	// The actual texture stored on the GPU side (cubemap)
		ID3D11ShaderResourceView* mapSRV;	// The texture in the shader resource view format, to send it to the shader
		ID3D11Texture2D* depthStencilMap;	// The depth stencil texture 
		ID3D11DepthStencilView* depthStencilView;	// The depth stencil view to set it as the render target
		ID3D11RenderTargetView* RTV[6];	// The 6 different render targets, one for each face, to bind to the render target
		UINT size;	// Size of each face of the cubemap

		// Getters and Setters for the size
		auto Size() { return size; }
		void SetSize(UINT s);

		// It releases the textures
		void Release();
	} mAmbientMap;

	// World matrices for the model
	// Now that meshes have multiple parts, we need multiple matrices. The root matrix (the first one) is the world matrix
	// for the entire model. The remaining matrices are relative to their parent part. The hierarchy is defined in the mesh (nodes)
	std::vector<CMatrix4x4> mWorldMatrices;
};
