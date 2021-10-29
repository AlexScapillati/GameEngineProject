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

#include <array>
#include <list>
#include <stdexcept>
#include <utility>
#include <wrl.h>

using namespace Microsoft::WRL;

class IScene
{
public:
	

	void InitTextures();

	IScene(CDX11Engine* engine, std::string fileName);

	void RenderSceneFromCamera(CCamera* camera);

	//--------------------------------------------------------------------------------------
	// Scene Render and Update
	//--------------------------------------------------------------------------------------

	// Returns the generated scene texture
	void RenderScene(float& frameTime);

	// frameTime is the time passed since the last frame
	void UpdateScene(float& frameTime);

	//--------------------------------------------------------------------------------------
	// Public Variables TODO REMOVE BIG NONO
	//--------------------------------------------------------------------------------------

	std::string mDefaultVs;
	std::string mDefaultPs;

	std::unique_ptr<CGameObjectManager> mObjManager;

	std::unique_ptr<CCamera> mCamera;

	// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
	bool mLockFPS = true;

	ColourRGBA mBackgroundColor;

	UINT mViewportX;
	UINT mViewportY;

	int mPcfSamples;

	//--------------------------------------------------------------------------------------
	// Public Functions
	//--------------------------------------------------------------------------------------

	void Save(std::string fileName = "");
	void Resize(UINT newX, UINT newY);
	void PostProcessingPass();
	void RenderToDepthMap();
	void DisplayPostProcessingEffects(); // TODO: Remove
	

	//--------------------------------------------------------------------------------------
	// Getters 
	//--------------------------------------------------------------------------------------


	auto GetObjectManager() const
	{
		return mObjManager.get();
	}

	auto GetViewportSize() const
	{
		return CVector2((float)mViewportX, (float)mViewportY);
	}

	auto GetViewportX() const
	{
		return mViewportX;
	}

	auto GetViewportY() const
	{
		return mViewportY;
	}

	auto GetCamera() const
	{
		return mCamera.get();
	}

	auto& GetLockFps()
	{
		return mLockFPS;
	}

	auto GetTextureSRV()
	{
		return mSceneSRV.Get();
	}

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

		std::string mShaderFileName;		//not used
		ComPtr < ID3D11PixelShader> shader = nullptr;	//not used

		ComPtr < ID3D11Texture2D> tex = nullptr;			//not used
		ComPtr < ID3D11ShaderResourceView> texSRV = nullptr;	//not used
	};

	std::list<PostProcessFilter> mPostProcessingFilters;

private:

	//--------------------------------------------------------------------------------------
	// Scene Data
	//--------------------------------------------------------------------------------------

	CDX11Engine* mEngine = nullptr;
	CWindow* mWindow = nullptr;

	std::string mFileName;

	// Variables controlling light1's orbiting of the particle emitter
	float gCameraOrbitRadius;
	float gCameraOrbitSpeed;

	// Additional light information
	CVector3 gAmbientColour; // Background level of light (slightly bluish to match the far background, which is dark blue)
	float    gSpecularPower; // Specular power controls shininess - same for all models in this app


	CGameObject* mSelectedObj;

	ComPtr<ID3D11Texture2D> mTextrue;
	ComPtr<ID3D11ShaderResourceView> mSceneSRV;
	ComPtr<ID3D11RenderTargetView> mSceneRTV;

	ComPtr<ID3D11Texture2D> mDepthStencil;
	ComPtr<ID3D11ShaderResourceView> mDepthStencilSRV;
	ComPtr<ID3D11DepthStencilView> mDepthStencilRTV;

	ComPtr<ID3D11Texture2D> mFinalTextrue;
	ComPtr<ID3D11ShaderResourceView> mFinalTextureSRV;
	ComPtr<ID3D11RenderTargetView> mFinalRTV;

	ComPtr<ID3D11Texture2D> mFinalDepthStencil;
	ComPtr<ID3D11ShaderResourceView> mFinalDepthStencilSRV;
	ComPtr<ID3D11DepthStencilView> mFinalDepthStencilRTV;

	//****************************
	// Post processing textures

	// Additional textures used for specific post-processes
	ComPtr<ID3D11Resource> mNoiseMap = nullptr;
	ComPtr<ID3D11ShaderResourceView> mNoiseMapSRV = nullptr;

	ComPtr<ID3D11Resource> mBurnMap = nullptr;
	ComPtr<ID3D11ShaderResourceView> mBurnMapSRV = nullptr;

	ComPtr<ID3D11Resource> mDistortMap = nullptr;
	ComPtr<ID3D11ShaderResourceView> mDistortMapSRV = nullptr;

	ComPtr<ID3D11Texture2D> mLuminanceMap = nullptr;
	ComPtr<ID3D11ShaderResourceView> mLuminanceMapSRV = nullptr;
	ComPtr<ID3D11RenderTargetView> mLuminanceRTV = nullptr;

	ComPtr<ID3D11Resource> mRandomMap = nullptr;
	ComPtr<ID3D11ShaderResourceView> mRandomMapSRV = nullptr;

	bool mSsaoBlur = false;

	ComPtr<ID3D11Texture2D > mSsaoMap = nullptr;
	ComPtr<ID3D11ShaderResourceView > mSsaoMapSRV = nullptr;
	ComPtr<ID3D11RenderTargetView > mSsaoMapRTV = nullptr;

	//****************************

	//--------------------------------------------------------------------------------------
	// PostProcess Functions
	//--------------------------------------------------------------------------------------

	// Select the appropriate shader plus any additional textures required for a given post-process
	// Helper function shared by full-screen, area and polygon post-processing functions below
	void SelectPostProcessShaderAndTextures(IScene::PostProcess postProcess);

	// Perform a full-screen post process from "scene texture" to back buffer
	void FullScreenPostProcess(PostProcess postProcess);

	// Perform an area post process from "scene texture" to back buffer at a given point in the world, with a given size (world units)
	void AreaPostProcess(PostProcess postProcess, CVector3 worldPoint, CVector2 areaSize);

	// Perform an post process from "scene texture" to back buffer within the given four-point polygon and a world matrix to position/rotate/scale the polygon
	void PolygonPostProcess(PostProcess postProcess, const std::array<CVector3, 4>& points, const CMatrix4x4& worldMatrix);

	//to remove
	void LoadPostProcessingImages();

	void ReleasePostProcessingShaders();
};