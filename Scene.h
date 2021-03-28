//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#pragma once

#include "Camera.h"
#include "GameObjectManager.h"

#include "CVector3.h"
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here
#include "ColourRGBA.h"
#include "tinyxml2/tinyxml2.h"

#include <sstream>
#include <array>
#include <list>
#include <stdexcept>
#include <utility>

class CScene
{
public:

	CScene(std::string fileName);

	void InitTextures();

	void RenderSceneFromCamera(CCamera* camera);

	//--------------------------------------------------------------------------------------
	// Scene Render and Update
	//--------------------------------------------------------------------------------------

	ID3D11ShaderResourceView* RenderScene(float frameTime);

	// frameTime is the time passed since the last frame
	void UpdateScene(float frameTime);

	//--------------------------------------------------------------------------------------
	// GUI functions
	//--------------------------------------------------------------------------------------

	void AddObjectsMenu();

	void DisplayPropertiesWindow();

	void DisplayObjects();

	void DisplaySceneSettings(bool& b);

	//--------------------------------------------------------------------------------------
	// Public Variables TODO REMOVE BIG NONO
	//--------------------------------------------------------------------------------------

	std::string mDefaultVs;
	std::string mDefaultPs;

	CGameObjectManager* mObjManager;

	CCamera* mCamera;

	// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
	bool mLockFPS = true;

	ColourRGBA mBackgroundColor;

	UINT mViewportX;
	UINT mViewportY;

	int mPcfSamples;

	void Save(std::string fileName = "");

	~CScene();

	void ReleaseTextures();

	void ReleaseSRVandRTVs();

	void Resize(UINT newX, UINT newY);

	void PostProcessingPass();

	void RenderToDepthMap();

	void DisplayPostProcessingEffects();

	//********************
	// Available post-processes

	enum class PostProcess
	{
		None,
		Copy,
		Tint,
		GreyNoise,
		Burn,
		Distort,
		Spiral,
		HeatHaze,
		ChromaticAberration,
		GaussionBlur,
		SSAO,
		Bloom,
		GodRays
	};

	std::string mPostProcessStrings[13] =
	{
		"None",
		"Copy",
		"Tint",
		"GreyNoise",
		"Burn",
		"Distort",
		"Spiral",
		"HeatHaze",
		"ChromaticAberration",
		"GaussionBlur",
		"SSAO",
		"Bloom",
		"GodRays"
	};

	enum class PostProcessMode
	{
		Fullscreen,
		Area,
		Polygon,
	};

	std::string mPostProcessModeStrings[3] =
	{
		"FullScreen",
		"Area",
		"Polygon"
	};

	//********************

	struct PostProcessFilter
	{
		PostProcess type = PostProcess::None;
		PostProcessMode mode = PostProcessMode::Fullscreen;

		std::string shaderFileName = "";		//not used
		ID3D11PixelShader* shader = nullptr;	//not used

		ID3D11Texture2D* tex = nullptr;			//not used
		ID3D11ShaderResourceView* texSRV = nullptr;	//not used
	};

	std::list<PostProcessFilter> mPostProcessingFilters;

private:

	//--------------------------------------------------------------------------------------
	// Scene Data
	//--------------------------------------------------------------------------------------

	std::string mFileName;

	// Variables controlling light1's orbiting of the particle emitter
	float gCameraOrbitRadius;
	float gCameraOrbitSpeed;

	// Additional light information
	CVector3 gAmbientColour; // Background level of light (slightly bluish to match the far background, which is dark blue)
	float    gSpecularPower; // Specular power controls shininess - same for all models in this app

	// Variables controlling light1's orbiting of the cube
	float gLightOrbitRadius;
	float gLightOrbitSpeed;

	CGameObject* mSelectedObj;

	ID3D11Texture2D* mTextrue;
	ID3D11ShaderResourceView* mSceneSRV;
	ID3D11RenderTargetView* mSceneRTV;

	ID3D11Texture2D* mDepthStencil;
	ID3D11ShaderResourceView* mDepthStencilSRV;
	ID3D11DepthStencilView* mDepthStencilRTV;

	ID3D11Texture2D* mFinalTextrue;
	ID3D11ShaderResourceView* mFinalTextureSRV;
	ID3D11RenderTargetView* mFinalRTV;

	ID3D11Texture2D* mFinalDepthStencil;
	ID3D11ShaderResourceView* mFinalDepthStencilSRV;
	ID3D11DepthStencilView* mFinalDepthStencilRTV;

	//****************************
	// Post processing textures

	// Additional textures used for specific post-processes
	ID3D11Resource* mNoiseMap = nullptr;
	ID3D11ShaderResourceView* mNoiseMapSRV = nullptr;

	ID3D11Resource* mBurnMap = nullptr;
	ID3D11ShaderResourceView* mBurnMapSRV = nullptr;

	ID3D11Resource* mDistortMap = nullptr;
	ID3D11ShaderResourceView* mDistortMapSRV = nullptr;

	ID3D11Texture2D* mLuminanceMap = nullptr;
	ID3D11ShaderResourceView* mLuminanceMapSRV = nullptr;
	ID3D11RenderTargetView* mLuminanceRTV = nullptr;

	ID3D11Resource* mRandomMap = nullptr;
	ID3D11ShaderResourceView* mRandomMapSRV = nullptr;

	bool mSsaoBlur = false;

	ID3D11Texture2D* mSsaoMap = nullptr;
	ID3D11ShaderResourceView* mSsaoMapSRV = nullptr;
	ID3D11RenderTargetView* mSsaoMapRTV = nullptr;

	//****************************

	//--------------------------------------------------------------------------------------
	// PostProcess Functions
	//--------------------------------------------------------------------------------------

	// Select the appropriate shader plus any additional textures required for a given post-process
	// Helper function shared by full-screen, area and polygon post-processing functions below
	void SelectPostProcessShaderAndTextures(CScene::PostProcess postProcess);

	// Perform a full-screen post process from "scene texture" to back buffer
	void FullScreenPostProcess(PostProcess postProcess);

	// Perform an area post process from "scene texture" to back buffer at a given point in the world, with a given size (world units)
	void AreaPostProcess(PostProcess postProcess, CVector3 worldPoint, CVector2 areaSize);

	// Perform an post process from "scene texture" to back buffer within the given four-point polygon and a world matrix to position/rotate/scale the polygon
	void PolygonPostProcess(PostProcess postProcess, const std::array<CVector3, 4>& points, const CMatrix4x4& worldMatrix);

	//to remove
	void LoadPostProcessingImages();

	void ReleasePostProcessingShaders();

	template<class T>
	void DisplayDeque(std::deque<T*>& deque);
};