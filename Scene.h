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
#include <stdexcept>
#include <utility>


class CScene
{

public:


	CScene(std::string fileName);

	void RenderSceneFromCamera(CCamera* camera);

	//--------------------------------------------------------------------------------------
	// Scene Render and Update
	//--------------------------------------------------------------------------------------

	ID3D11ShaderResourceView* RenderScene(float frameTime);

	// frameTime is the time passed since the last frame
	void UpdateScene(float frameTime);


	void DisplayObjects();

	//--------------------------------------------------------------------------------------
	// Public Variables TODO REMOVE BIG NONO
	//--------------------------------------------------------------------------------------

	std::string mDefaultVs;
	std::string mDefaultPs;

	std::unique_ptr<CGameObjectManager> mObjManager;

	std::unique_ptr<CCamera> mCamera;

	// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
	bool mLockFPS = true;

	ColourRGBA gBackgroundColor;

	UINT mViewportX;
	UINT mViewportY;

	~CScene();

	void Resize(UINT newX, UINT newY);

	void PostProcessingPass();

private:

	//--------------------------------------------------------------------------------------
	// Scene Data
	//--------------------------------------------------------------------------------------



	// Variables controlling light1's orbiting of the particle emitter
	float gCameraOrbitRadius;
	float gCameraOrbitSpeed;


	// Additional light information
	CVector3 gAmbientColour; // Background level of light (slightly bluish to match the far background, which is dark blue)
	float    gSpecularPower; // Specular power controls shininess - same for all models in this app


	// Variables controlling light1's orbiting of the cube
	float gLightOrbitRadius;
	float gLightOrbitSpeed;


	ID3D11Texture2D* mTextrue;
	ID3D11ShaderResourceView* mTextureSRV;
	ID3D11RenderTargetView* mTargetView;
	ID3D11Texture2D* mDepthStencil;
	ID3D11DepthStencilView* mDepthStencilView;

	
	ID3D11Texture2D* mFinalTextrue;
	ID3D11ShaderResourceView* mFinalTextureSRV;
	ID3D11RenderTargetView* mFinalTargetView;


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
		GaussionBlur
	} mCurrPostProcess;

	enum class PostProcessMode
	{
		Fullscreen,
		Area,
		Polygon,
	} mCurrPostProcessMode;

	//********************


	//****************************
	// Post processing textures

	// Additional textures used for specific post-processes
	ID3D11Resource*				mNoiseMap = nullptr;
	ID3D11ShaderResourceView*	mNoiseMapSRV = nullptr;
	ID3D11Resource*				mBurnMap = nullptr;
	ID3D11ShaderResourceView*	mBurnMapSRV = nullptr;
	ID3D11Resource*				mDistortMap = nullptr;
	ID3D11ShaderResourceView*	mDistortMapSRV = nullptr;

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

	void LoadPostProcessingImages();


};