//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#pragma once

#include "Scene.h"
#include <dxgidebug.h>
#include <utility>

#include "SpotLight.h"
#include "DirLight.h"
#include "PointLight.h"
#include "Plant.h"
#include "Sky.h"

#include "LevelImporter.h"
#include "External\imgui\imgui.h"
#include "External\imgui\ImGuizmo.h"
#include "External\imgui\FileBrowser\ImGuiFileBrowser.h"

float ROTATION_SPEED = 0.25f;
float MOVEMENT_SPEED = 50.0f;

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h
// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer* gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer* gPerModelConstantBuffer; // --"--

PerFrameLights gPerFrameLightsConstants;
ID3D11Buffer* gPerFrameLightsConstBuffer;

PerFrameSpotLights gPerFrameSpotLightsConstants;
ID3D11Buffer* gPerFrameSpotLightsConstBuffer;

PerFrameDirLights gPerFrameDirLightsConstants;
ID3D11Buffer* gPerFrameDirLightsConstBuffer;

PerFramePointLights gPerFramePointLightsConstants;
ID3D11Buffer* gPerFramePointLightsConstBuffer;

PostProcessingConstants gPostProcessingConstants;
ID3D11Buffer* gPostProcessingConstBuffer;

CGameObjectManager* GOM;

CScene::CScene(std::string fileName)
{
	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------

	mCamera = nullptr;
	mSelectedObj = nullptr;

	mTextrue = nullptr;
	mSceneRTV = nullptr;
	mSceneSRV = nullptr;
	mDepthStencil = nullptr;
	mDepthStencilRTV = nullptr;
	mDepthStencilSRV = nullptr;

	mFinalTextrue = nullptr;
	mFinalRTV = nullptr;
	mFinalTextureSRV = nullptr;
	mFinalDepthStencil = nullptr;
	mFinalDepthStencilRTV = nullptr;
	mFinalDepthStencilSRV = nullptr;

	mViewportX = 1024;
	mViewportY = 720;

	mPcfSamples = 4;

	mObjManager = new CGameObjectManager();

	GOM = mObjManager;

	gAmbientColour = { 0.03f, 0.03f, 0.04f };
	gSpecularPower = 256; // Specular power //will be removed since it will be dependent on the material
	mLockFPS = true;
	mBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };

	try
	{
		//create all the textures needed for the scene rendering
		InitTextures();
	}
	catch (const std::runtime_error& e)
	{
		throw std::runtime_error(e.what());
	}

	//--------------------------------------------------------------------------------------
	// Initialise scene geometry, constant buffers and states
	//--------------------------------------------------------------------------------------

	////--------------- Load meshes ---------------////

	// Load mesh geometry data

	try
	{
		//load default shaders
		LoadDefaultShaders();

		LoadPostProcessingImages();

		//load the scene
		CLevelImporter importer;

		importer.LoadScene(std::move(fileName), this);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(e.what());
	}

	if (mObjManager->mObjects.empty())
	{
		throw std::runtime_error("No objects loaded");
	}

	////--------------- GPU states ---------------////

	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		throw std::runtime_error("Error creating DirectX states");
	}

	// Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
	// These allow us to pass data from CPU to shaders such as lighting information or matrices
	// See the comments above where these variable are declared and also the UpdateScene function
	gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
	gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
	gPerFrameLightsConstBuffer = CreateConstantBuffer(sizeof(gPerFrameLightsConstants));
	gPerFrameSpotLightsConstBuffer = CreateConstantBuffer(sizeof(gPerFrameSpotLightsConstants));
	gPerFrameDirLightsConstBuffer = CreateConstantBuffer(sizeof(gPerFrameDirLightsConstants));
	gPerFramePointLightsConstBuffer = CreateConstantBuffer(sizeof(gPerFramePointLightsConstants));
	gPostProcessingConstBuffer = CreateConstantBuffer(sizeof(gPostProcessingConstants));

	if (!gPerFrameConstantBuffer ||
		!gPerModelConstantBuffer ||
		!gPerFrameDirLightsConstBuffer ||
		!gPerFrameLightsConstBuffer ||
		!gPerFrameSpotLightsConstBuffer ||
		!gPerFramePointLightsConstBuffer ||
		!gPostProcessingConstBuffer)
	{
		throw std::runtime_error("Error creating constant buffers");
	}
}

//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render everything in the scene from the given camera
void CScene::RenderSceneFromCamera(CCamera* camera)
{
	// Set camera matrices in the constant buffer and send over to GPU
	gPerFrameConstants.cameraMatrix = camera->WorldMatrix();
	gPerFrameConstants.viewMatrix = camera->ViewMatrix();
	gPerFrameConstants.projectionMatrix = camera->ProjectionMatrix();
	gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();

	// Update the frame constant buffer
	UpdateFrameConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

	// Set it to the GPU
	gD3DContext->PSSetConstantBuffers(1, 1, &gPerFrameConstantBuffer);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPerFrameConstantBuffer);

	////--------------- Render ordinary models ---------------///

	// Select which shaders to use next
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  ////// Switch off geometry shader when not using it (pass nullptr for first parameter)

	// States - no blending, normal depth buffer and back-face culling (standard set-up for opaque models)
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	gD3DContext->RSSetState(gCullBackState);

	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

	//Render All Objects, if something went wrong throw an exception
	if (!mObjManager->RenderAllObjects())
	{
		throw std::exception("Could not render objects");
	}
}

// Rendering the scene
ID3D11ShaderResourceView* CScene::RenderScene(float frameTime)
{
	//// Common settings ////

	// Set up the light information in the constant buffer
	// Don't send to the GPU yet, the function RenderSceneFromCamera will do that

	mObjManager->UpdateAllBuffers();

	gPerFrameConstants.ambientColour = gAmbientColour;
	gPerFrameConstants.specularPower = gSpecularPower;
	gPerFrameConstants.cameraPosition = mCamera->Position();
	gPerFrameConstants.frameTime = frameTime;
	gPerFrameConstants.nPcfSamples = mPcfSamples;

	// Update constant buffest
	UpdateLightContantBuffer(gPerFrameLightsConstBuffer, gPerFrameLightsConstants, mObjManager->mLights.size());
	UpdateDirLightsContantBuffer(gPerFrameDirLightsConstBuffer, gPerFrameDirLightsConstants, mObjManager->mDirLights.size());
	UpdateSpotLightsContantBuffer(gPerFrameSpotLightsConstBuffer, gPerFrameSpotLightsConstants, mObjManager->mSpotLights.size());
	UpdatePointLightsContantBuffer(gPerFramePointLightsConstBuffer, gPerFramePointLightsConstants, mObjManager->mPointLights.size());

	// Set them to the GPU
	ID3D11Buffer* frameCBuffers[] =
	{
		gPerFrameLightsConstBuffer,
		gPerFrameSpotLightsConstBuffer,
		gPerFrameDirLightsConstBuffer,
		gPerFramePointLightsConstBuffer
	};

	gD3DContext->PSSetConstantBuffers(2, 4, frameCBuffers);
	gD3DContext->VSSetConstantBuffers(2, 4, frameCBuffers);

	// Set the sampler for the material textures
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

	// Set Sampler for the Shadow Maps
	gD3DContext->PSSetSamplers(1, 1, &gPointSamplerBorder);

	////----- Render form the lights point of view ----------////

	mObjManager->RenderFromAllLights();

	//if the shadowmaps array is not empty
	if (!mObjManager->mShadowsMaps.empty())
	{
		//send the shadow maps to the shaders (slot 7)
		gD3DContext->PSSetShaderResources(7, mObjManager->mShadowsMaps.size(), &mObjManager->mShadowsMaps[0]);
	}

	////--------------- Render Ambient Maps  ---------------////

	mObjManager->RenderAmbientMaps();

	////--------------- Main scene rendering ---------------////

	// Set the target for rendering and select the main depth buffer.
	// If using post-processing then render to the scene texture, otherwise to the usual back buffer
	// Also clear the render target to a fixed colour and the depth buffer to the far distance

	gD3DContext->OMSetRenderTargets(1, &mSceneRTV, mDepthStencilRTV);
	gD3DContext->ClearRenderTargetView(mSceneRTV, &mBackgroundColor.r);

	gD3DContext->ClearDepthStencilView(mDepthStencilRTV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Setup the viewport to the size of the main window
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(mViewportX);
	vp.Height = static_cast<FLOAT>(mViewportY);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gD3DContext->RSSetViewports(1, &vp);

	// Render the scene from the main camera
	RenderSceneFromCamera(mCamera);

	// Render the scene to a depth map, for post processing
	RenderToDepthMap();

	// PostProcessing pass
	PostProcessingPass();

	return mSceneSRV;
}

//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void CScene::UpdateScene(float frameTime)
{
	// Post processing settings - all data for post-processes is updated every frame whether in use or not (minimal cost)

	// Set and increase the burn level (cycling back to 0 when it reaches 1.0f)
	const float burnSpeed = 0.2f;
	gPostProcessingConstants.burnHeight = fmod(gPostProcessingConstants.burnHeight + burnSpeed * frameTime, 1.0f);

	// Set and increase the amount of spiral - use a tweaked cos wave to animate
	static float wiggle = 0.0f;
	const float wiggleSpeed = 1.0f;
	gPostProcessingConstants.spiralLevel = ((1.0f - cos(wiggle)) * 4.0f);
	wiggle += wiggleSpeed * frameTime;

	// Update heat haze timer
	gPostProcessingConstants.heatHazeTimer += frameTime;

	// Camera control 
	if (ImGui::IsWindowFocused())
	{
		if (KeyHeld(Mouse_RButton))
		{
			POINT mousePos;

			GetCursorPos(&mousePos);

			CVector2 delta = { ImGui::GetMouseDragDelta(1).x, ImGui::GetMouseDragDelta(1).y };

			if (KeyHeld(Key_LShift))
				MOVEMENT_SPEED = 100.0f;
			else
				MOVEMENT_SPEED = 50.0f;

			RECT winRect;

			GetWindowRect(gHWnd, &winRect);

			if (mousePos.x > winRect.right) SetCursorPos(winRect.left, mousePos.y);

			else if (mousePos.x < winRect.left) SetCursorPos(winRect.right, mousePos.y);

			else if (mousePos.y > winRect.bottom) SetCursorPos(mousePos.x, winRect.top);

			else if (mousePos.y < winRect.top) SetCursorPos(mousePos.x, winRect.bottom);

			else
				mCamera->ControlMouse(frameTime , delta, Key_W, Key_S, Key_A, Key_D);

			ImGui::ResetMouseDragDelta(1);
		}
	}

	// Toggle FPS limiting
	if (KeyHit(Key_P))
	{
		mLockFPS = !mLockFPS;
	}

	// Show frame time / FPS in the window title //
	const auto fpsUpdateTime = 0.5f; // How long between updates (in seconds)
	static float totalFrameTime = 0;
	static auto frameCount = 0;
	totalFrameTime += frameTime;
	++frameCount;
	if (totalFrameTime > fpsUpdateTime)
	{
		// Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
		const auto avgFrameTime = totalFrameTime / frameCount;
		std::ostringstream frameTimeMs;
		frameTimeMs.precision(2);
		frameTimeMs << std::fixed << avgFrameTime * 1000;
		const auto windowTitle = "DirectX 11 - Game Engine Test " + frameTimeMs.str() +
			"ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
		SetWindowTextA(gHWnd, windowTitle.c_str());
		totalFrameTime = 0;
		frameCount = 0;
	}
}

void CScene::Save(std::string fileName)
{
	CLevelImporter importer;

	importer.SaveScene(fileName, this);
}

CScene::~CScene()
{
	delete mObjManager; mObjManager = nullptr;
	delete mCamera;		mCamera = nullptr;

	if (gPerModelConstantBuffer)			gPerModelConstantBuffer->Release();			gPerModelConstantBuffer = nullptr;
	if (gPerFrameConstantBuffer)			gPerFrameConstantBuffer->Release();			gPerFrameConstantBuffer = nullptr;
	if (gPerFrameLightsConstBuffer)			gPerFrameLightsConstBuffer->Release();		gPerFrameLightsConstBuffer = nullptr;
	if (gPerFrameSpotLightsConstBuffer)		gPerFrameSpotLightsConstBuffer->Release();	gPerFrameSpotLightsConstBuffer = nullptr;
	if (gPerFrameDirLightsConstBuffer)		gPerFrameDirLightsConstBuffer->Release();	gPerFrameDirLightsConstBuffer = nullptr;
	if (gPerFramePointLightsConstBuffer)	gPerFramePointLightsConstBuffer->Release();	gPerFramePointLightsConstBuffer = nullptr;
	if (gPostProcessingConstBuffer)			gPostProcessingConstBuffer->Release();		gPostProcessingConstBuffer = nullptr;

	// Release all the texture that are vieport size dependent
	ReleaseTextures();

	ReleaseSRVandRTVs();
	
	ReleasePostProcessingShaders();

	ReleaseDefaultShaders();
}

void CScene::ReleaseTextures()
{
	if (mTextrue)							mTextrue->Release();						mTextrue = nullptr;
	if (mFinalTextrue)						mFinalTextrue->Release();					mFinalTextrue = nullptr;
	if (mDepthStencil)						mDepthStencil->Release();					mDepthStencil = nullptr;
	if (mSsaoMap)							mSsaoMap->Release();						mSsaoMap = nullptr;
	if (mLuminanceMap)						mLuminanceMap->Release();					mLuminanceMap = nullptr;
	if (mFinalDepthStencil)					mFinalDepthStencil->Release();				mFinalDepthStencil = nullptr;
}

void CScene::ReleaseSRVandRTVs()
{
	//Release the SRV and RTV
	if (mSceneRTV)							mSceneRTV->Release();						mSceneRTV = nullptr;
	if (mSceneSRV)							mSceneSRV->Release();						mSceneSRV = nullptr;

	if (mDepthStencilRTV)					mDepthStencilRTV->Release();				mDepthStencilRTV = nullptr;
	if (mDepthStencilSRV)					mDepthStencilSRV->Release();				mDepthStencilSRV = nullptr;

	if (mFinalRTV)							mFinalRTV->Release();						mFinalRTV = nullptr;
	if (mFinalTextureSRV)					mFinalTextureSRV->Release();				mFinalTextureSRV = nullptr;

	if (mFinalDepthStencilSRV)				mFinalDepthStencilSRV->Release();			mFinalDepthStencilSRV = nullptr;
	if (mFinalDepthStencilRTV)				mFinalDepthStencilRTV->Release();			mFinalDepthStencilRTV = nullptr;

	if (mLuminanceMapSRV)					mLuminanceMapSRV->Release();				mLuminanceMapSRV = nullptr;
	if (mLuminanceRTV)						mLuminanceRTV->Release();					mLuminanceRTV = nullptr;

	if (mSsaoMapRTV)						mSsaoMapRTV->Release();						mSsaoMapRTV = nullptr;
	if (mSsaoMapSRV)						mSsaoMapSRV->Release();						mSsaoMapSRV = nullptr;

}


void CScene::Resize(UINT newX, UINT newY)
{
	//set aspect ratio with the new window size
	//broken
	mCamera->SetAspectRatio(float(newX) / float(newY));

	// set the scene viewport size to the new size
	mViewportX = newX;
	mViewportY = newY;

	// Release all the texture that are vieport size dependent
	ReleaseTextures();

	// Release the SRV and RTVs
	ReleaseSRVandRTVs();

	// Recreate all the texture with the updated size
	InitTextures();
}

void CScene::InitTextures()
{
	// We create a new texture for the scene with new size
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = mViewportX; // Size of the "screen"
	textureDesc.Height = mViewportY;
	textureDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Indicate we will use texture as a depth buffer and also pass it to shaders
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &mTextrue)))
	{
		throw std::runtime_error("Error creating scene texture");
	}

	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &mFinalTextrue)))
	{
		throw std::runtime_error("Error creating scene texture");
	}

	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &mLuminanceMap)))
	{
		throw std::runtime_error("Error creating luminance texture");
	}

	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &mSsaoMap)))
	{
		throw std::runtime_error("Error creating ssao texture");
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	// We created the scene texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(gD3DDevice->CreateRenderTargetView(mTextrue, &rtvDesc, &mSceneRTV)))
	{
		gLastError = "Error creating scene render target view";
	}

	if (FAILED(gD3DDevice->CreateRenderTargetView(mFinalTextrue, &rtvDesc, &mFinalRTV)))
	{
		gLastError = "Error creating scene render target view";
	}

	if (FAILED(gD3DDevice->CreateRenderTargetView(mLuminanceMap, &rtvDesc, &mLuminanceRTV)))
	{
		gLastError = "Error creating luminance render target view";
	}

	if (FAILED(gD3DDevice->CreateRenderTargetView(mSsaoMap, &rtvDesc, &mSsaoMapRTV)))
	{
		gLastError = "Error creating ssao render target view";
	}

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	if (FAILED(gD3DDevice->CreateShaderResourceView(mTextrue, &srvDesc, &mSceneSRV)))
	{
		throw std::runtime_error("Error creating scene texture shader resource view");
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(mFinalTextrue, &srvDesc, &mFinalTextureSRV)))
	{
		throw std::runtime_error("Error creating scene texture shader resource view");
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(mLuminanceMap, &srvDesc, &mLuminanceMapSRV)))
	{
		throw std::runtime_error("Error creating luminance shader resource view");
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(mSsaoMap, &srvDesc, &mSsaoMapSRV)))
	{
		throw std::runtime_error("Error creating ssao shader resource view");
	}

	//create depth stencil
	D3D11_TEXTURE2D_DESC dsDesc = {};
	dsDesc.Width = textureDesc.Width;
	dsDesc.Height = textureDesc.Height;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	if (FAILED(gD3DDevice->CreateTexture2D(&dsDesc, NULL, &mDepthStencil)))
	{
		throw std::runtime_error("Error creating depth stencil");
	}

	if (FAILED(gD3DDevice->CreateTexture2D(&dsDesc, NULL, &mFinalDepthStencil)))
	{
		throw std::runtime_error("Error creating depth stencil");
	}

	//create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(gD3DDevice->CreateDepthStencilView(mDepthStencil, &dsvDesc, &mDepthStencilRTV)))
	{
		throw std::runtime_error("Error creating depth stencil view ");
	}

	if (FAILED(gD3DDevice->CreateDepthStencilView(mFinalDepthStencil, &dsvDesc, &mFinalDepthStencilRTV)))
	{
		throw std::runtime_error("Error creating depth stencil view ");
	}

	//create the depth stencil shader view

	D3D11_SHADER_RESOURCE_VIEW_DESC dsSrvDesc = {};
	dsSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	dsSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dsSrvDesc.Texture2D.MostDetailedMip = 0;
	dsSrvDesc.Texture2D.MipLevels = -1;

	if (FAILED(gD3DDevice->CreateShaderResourceView(mDepthStencil, &dsSrvDesc, &mDepthStencilSRV)))
	{
		throw std::runtime_error("Error creating depth stencil shader resource view");
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(mFinalDepthStencil, &dsSrvDesc, &mFinalDepthStencilSRV)))
	{
		throw std::runtime_error("Error creating depth stencil shader resource view");
	}

	// Now relese the textures since we do not need them anymore
	ReleaseTextures();
}