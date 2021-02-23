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


const float ROTATION_SPEED = 1.5f;
const float MOVEMENT_SPEED = 50.0f;

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



//*******************************
//**** Post-processing shader DirectX objects
// These are also added to Shader.h
extern ID3D11VertexShader* g2DQuadVertexShader;
extern ID3D11VertexShader* g2DPolygonVertexShader;
extern ID3D11PixelShader* gCopyPostProcess;
extern ID3D11PixelShader* gTintPostProcess;
extern ID3D11PixelShader* gGreyNoisePostProcess;
extern ID3D11PixelShader* gBurnPostProcess;
extern ID3D11PixelShader* gDistortPostProcess;
extern ID3D11PixelShader* gSpiralPostProcess;
extern ID3D11PixelShader* gHeatHazePostProcess;
extern ID3D11PixelShader* gChromaticAberrationPostProcess;
extern ID3D11PixelShader* gGaussionBlurPostProcess;

CGameObjectManager* GOM;

void CScene::DisplayObjects()
{

	static CGameObject* selectedObj = nullptr;

	static auto mCurrentGizmoOperation = ImGuizmo::TRANSLATE;

	static bool showBounds = false;

	if (ImGui::Begin("Objects"))
	{
		//	static bool addObj = false;

		//	if (ImGui::Button("Add Object"))
		//	{
		//		addObj = true;
		//	}

		//	if (ImGui::Begin("Add object"), &addObj)
		//	{
		//		//WIP
		//		ImGui::Button("Add Mesh");
		//		ImGui::Button("Add Texture");

		//	}
		//	ImGui::End();

		ImGui::Separator();
		ImGui::Separator();

		//display for each model a button
		for (auto it : GOM->mObjects)
		{
			//if a butto is pressed
			if (ImGui::Button(it->GetName().c_str()))
			{
				//stor the object pointer
				selectedObj = it;
			}
		}

		for (auto it : GOM->mLights)
		{
			//if a butto is pressed
			if (ImGui::Button(it->GetName().c_str()))
			{
				//stor the object pointer
				selectedObj = it;
			}
		}

		for (auto it : GOM->mDirLights)
		{
			//if a butto is pressed
			if (ImGui::Button(it->GetName().c_str()))
			{
				//stor the object pointer
				selectedObj = it;
			}
		}

		for (auto it : GOM->mSpotLights)
		{
			//if a butto is pressed
			if (ImGui::Button(it->GetName().c_str()))
			{
				//stor the object pointer
				selectedObj = it;
			}
		}

		for (auto it : GOM->mPointLights)
		{
			//if a butto is pressed
			if (ImGui::Button(it->GetName().c_str()))
			{
				//stor the object pointer
				selectedObj = it;
			}
		}

		//if there is an object selected
		if (selectedObj)
		{
			ImGuizmo::Enable(selectedObj->Enabled());

			ImGui::Separator();

			if (ImGui::Button("Duplicate Selected Obj NOT WORKING"))
			{
				//WIP
				try
				{
					auto obj = new CGameObject(*selectedObj);

					GOM->AddObject(obj);
				}
				catch (std::exception& e)
				{
					throw std::runtime_error(e.what());
				}

			}

			ImGui::Checkbox("Enabled", selectedObj->Enabled());

			ImGui::Checkbox("Toggle ambient Map", selectedObj->AmbientMapEnabled());

			//display the transform component
			ImGui::NewLine();
			ImGui::Separator();

			ImGui::Text("Transform");

			if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
				mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
				mCurrentGizmoOperation = ImGuizmo::ROTATE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
				mCurrentGizmoOperation = ImGuizmo::SCALE;

			//get the direct access to the position of the model and display it 
			ImGui::DragFloat3("Position", selectedObj->DirectPosition());

			//acquire the rotation array
			float* rot = selectedObj->Rotation().GetValuesArray();

			//convert it to degreese
			for (int i = 0; i < 3; i++)
			{
				rot[i] = ToDegrees(rot[i]);
			}

			//display the rotation
			if (ImGui::DragFloat3("Rotation", rot))
			{
				//if the value is changed 
				//get back to radians
				for (int i = 0; i < 3; i++)
				{
					rot[i] = ToRadians(rot[i]);
				}

				//set the rotation
				selectedObj->SetRotation(rot);
			}

			//get the scale array
			float* scale = selectedObj->Scale().GetValuesArray();

			//display the scale array
			if (ImGui::DragFloat3("Scale", scale, 0.1f, 0.001f, D3D11_FLOAT32_MAX))
			{
				//if it has changed set the scale
				selectedObj->SetScale(scale);
			}


			ImGui::Checkbox("Show Bounds", &showBounds);

			//----------------------------------------------------------------
			// Object Specific settings
			//----------------------------------------------------------------

			if (auto light = dynamic_cast<CLight*>(selectedObj))
			{

				ImGui::Text("Specific settings");

				//light colour edit

				static bool colourPickerOpen = false;

				if (ImGui::Button("Edit Colour"))
				{
					colourPickerOpen = !colourPickerOpen;
				}

				if (colourPickerOpen)
				{
					if (ImGui::Begin("ColourPicker", &colourPickerOpen))
					{
						ImGui::ColorPicker3("Colour", light->GetColour().GetValuesArray());
					}
					ImGui::End();
				}

				//modify strength
				ImGui::DragFloat("Strength", &light->GetStrength(), 1.0f, 0.0f, D3D11_FLOAT32_MAX);

				//if it is a spotlight let it modify few things
				if (auto spotLight = dynamic_cast<CSpotLight*>(selectedObj))
				{
					//modify facing
					CVector3 facingV = spotLight->GetFacing();

					if (ImGui::DragFloat3("Facing", facingV.GetValuesArray(), 0.001f, -1.0f, 1.0f))
					{
						CVector3 facing = Normalise(facingV);
						spotLight->SetFacing(facing);
					}


					//modify cone angle
					ImGui::DragFloat("Cone Angle", &spotLight->GetConeAngle(), 1.0f, 0.0f, 180.0f);

					//modify shadow map size
					ImGui::DragInt("ShadowMapsSize", &spotLight->getShadowMapSize(), 1.0f, 2, 16384);

				}
				else if (auto dirLight = dynamic_cast<CDirLight*>(selectedObj))
				{
					//modify direction

					auto facingV = dirLight->GetDirection();

					if (ImGui::DragFloat3("Facing", facingV.GetValuesArray(), 0.001f, -1.0f, 1.0f))
					{
						CVector3 facing = Normalise(facingV);
						dirLight->SetDirection(facing);
					}
					//modify shadow map size

					auto size = dirLight->GetShadowMapSize();

					if (ImGui::DragInt("ShadowMapsSize", &size, 1.0f, 2, 16384))
					{
						if (size < 16384 && size >2)
						{
							dirLight->SetShadowMapSize(size);
						}
						else
						{
							std::runtime_error("Number Too big!");
						}
					}

					//modify near clip and far clip

					auto nearClip = dirLight->GetNearClip();
					auto farClip = dirLight->GetFarClip();

					if (ImGui::DragFloat("NearClip", &nearClip, 0.01f, 0.0f, 10.0f))
					{
						dirLight->SetNearClip(nearClip);
					}

					if (ImGui::DragFloat("FarClip", &farClip, 10.0f, 0.0f, D3D11_FLOAT32_MAX))
					{
						dirLight->SetFarClip(farClip);
					}
				}
			}


			//display the texture
			ImGui::NewLine();
			ImGui::Text("Texture");

			ImTextureID texId = selectedObj->GetTextureSRV();

			ImGui::Image((void*)texId, { 256,256 });

			//display the ambient map (if any)
			if (*selectedObj->AmbientMapEnabled())
			{
				ImGui::NewLine();
				ImGui::Text("AmbientMap");

				//ImGui::Image((void*)selectedObj->GetAmbientMap(), {256.f, 256.f});
			}
		}
	}
	ImGui::End();

	//display imguizmo outside the object window (therfore in the viewport window)
	if (selectedObj)
	{
		auto pos = ImGui::GetWindowPos();

		ImGuizmo::SetRect(pos.x, pos.y, pos.x + mViewportX, pos.y + mViewportY);

		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		static float bounds[] =
		{
			0.0f,0.0f,0.0f,
			 1.f, 1.f, 1.f
		};

		ImGuizmo::Manipulate(mCamera->ViewMatrix().GetArray(), mCamera->ProjectionMatrix().GetArray(),
			mCurrentGizmoOperation, ImGuizmo::WORLD, selectedObj->WorldMatrix().GetArray(), 0, 0, showBounds ? bounds : 0);
	}
}


void CScene::LoadPostProcessingImages()
{
	if (!LoadTexture("Noise.png", &mNoiseMap, &mNoiseMapSRV) ||
		!LoadTexture("Burn.png", &mBurnMap, &mBurnMapSRV) ||
		!LoadTexture("Distort.png", &mDistortMap, &mDistortMapSRV))
	{
		throw std::runtime_error("Error loading post processing images");
	}
}

CScene::CScene(std::string fileName)
{

	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------

	mCamera = nullptr;
	mTextrue = nullptr;
	mTargetView = nullptr;
	mTextureSRV = nullptr;
	mDepthStencil = nullptr;
	mDepthStencilView = nullptr;

	mViewportX = 1024;
	mViewportY = 720;

	mObjManager = std::make_unique<CGameObjectManager>();

	GOM = mObjManager.get();

	gCameraOrbitRadius = 60.0f;
	gCameraOrbitSpeed = 1.2f;
	gAmbientColour = { 0.3f, 0.3f, 0.4f };
	gSpecularPower = 256; // Specular power //will be removed since it will be dependent on the material
	mLockFPS = true;
	gBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };

	mCurrPostProcess = PostProcess::None;
	mCurrPostProcessMode = PostProcessMode::Fullscreen;

	// We also need a depth buffer to go with our texture
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

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	// We created the scene texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(gD3DDevice->CreateRenderTargetView(mTextrue, &rtvDesc, &mTargetView)))
	{
		gLastError = "Error creating scene render target view";
	}

	if (FAILED(gD3DDevice->CreateRenderTargetView(mFinalTextrue, &rtvDesc, &mFinalTargetView)))
	{
		gLastError = "Error creating scene render target view";
	}

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	if (FAILED(gD3DDevice->CreateShaderResourceView(mTextrue, &srvDesc, &mTextureSRV)))
	{
		throw std::runtime_error("Error creating scene texture shader resource view");
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(mFinalTextrue, &srvDesc, &mFinalTextureSRV)))
	{
		throw std::runtime_error("Error creating scene texture shader resource view");
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
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	//create it
	if (FAILED(gD3DDevice->CreateTexture2D(&dsDesc, NULL, &mDepthStencil)))
	{
		throw std::runtime_error("Error creating depth stencil");
	}

	//create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(gD3DDevice->CreateDepthStencilView(mDepthStencil, &dsvDesc, &mDepthStencilView)))
	{
		throw std::runtime_error("Error creating depth stencil view ");
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

	UpdateFrameConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);
	UpdateLightBuffer(gPerFrameLightsConstBuffer, gPerFrameLightsConstants);
	UpdateSpotLightsBuffer(gPerFrameSpotLightsConstBuffer, gPerFrameSpotLightsConstants);
	UpdateDirLightsBuffer(gPerFrameDirLightsConstBuffer, gPerFrameDirLightsConstants);
	UpdatePointLightsBuffer(gPerFramePointLightsConstBuffer, gPerFramePointLightsConstants);

	ID3D11Buffer* frameCBuffers[] = { gPerFrameConstantBuffer,
		gPerFrameLightsConstBuffer,
		gPerFrameSpotLightsConstBuffer,
		gPerFrameDirLightsConstBuffer,
		gPerFramePointLightsConstBuffer };

	gD3DContext->PSSetConstantBuffers(1, 5, frameCBuffers);
	gD3DContext->VSSetConstantBuffers(1, 5, frameCBuffers); // First parameter must match constant buffer number in the shader 
	gD3DContext->GSSetConstantBuffers(1, 5, frameCBuffers);

	////--------------- Render ordinary models ---------------///

	// Select which shaders to use next
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  ////// Switch off geometry shader when not using it (pass nullptr for first parameter)

	gD3DContext->PSSetSamplers(1, 1, &gPointSampler);

	gD3DContext->PSSetShaderResources(5, 0, nullptr);

	//if the shadowmaps array is not empty
	if (!mObjManager->mShadowsMaps.empty())
	{
		//send the shadow maps to the shaders (slot 6)
		gD3DContext->PSSetShaderResources(6, mObjManager->mShadowsMaps.size(), &mObjManager->mShadowsMaps[0]);
	}

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

	mObjManager->UpdateLightsConstBuffer(&gPerFrameLightsConstants);
	mObjManager->UpdateSpotLightsConstBuffer(&gPerFrameSpotLightsConstants);
	mObjManager->UpdateDirLightsConstBuffer(&gPerFrameDirLightsConstants);
	mObjManager->UpdatePointLightsConstBuffer(&gPerFramePointLightsConstants);

	gPerModelConstants.parallaxDepth = 0.00f;

	gPerFrameConstants.ambientColour = gAmbientColour;
	gPerFrameConstants.specularPower = gSpecularPower;
	gPerFrameConstants.cameraPosition = mCamera->Position();
	gPerFrameConstants.frameTime = frameTime;

	////----- Render form the lights point of view ----------////

	mObjManager->RenderFromAllLights();

	////--------------- Main scene rendering ---------------////

	// Set the target for rendering and select the main depth buffer.
	// If using post-processing then render to the scene texture, otherwise to the usual back buffer
	// Also clear the render target to a fixed colour and the depth buffer to the far distance

	//if (mCurrPostProcess != PostProcess::None)
	//{
	//	gD3DContext->OMSetRenderTargets(1, &mFinalTargetView, mDepthStencilView);
	//	gD3DContext->ClearRenderTargetView(mFinalTargetView, &gBackgroundColor.r);
	//}
	//else
	//{
	gD3DContext->OMSetRenderTargets(1, &mTargetView, mDepthStencilView);
	gD3DContext->ClearRenderTargetView(mTargetView, &gBackgroundColor.r);
	//}

	gD3DContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


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
	RenderSceneFromCamera(mCamera.get());

	//PostProcessing pass
	PostProcessingPass();

	if (mCurrPostProcess != PostProcess::None)
	{
		return mFinalTextureSRV;
	}
	else
	{
		return mTextureSRV;
	}
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

	//***********
	if (ImGui::IsWindowFocused())
	{
		mCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);
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

void CScene::PostProcessingPass()
{
	//show the postprocessing window
	if (!gViewportFullscreen)
	{
		//Render a window with all the postprocessing 
		if (ImGui::Begin("PostProcessing", 0))
		{
			//render two table 
			//one is for the type of PP 
			//one is for the mode

			auto items = "None\0Tint\0GrayNoise\0Burn\0Distort\0Spiral\0HeatHaze\0ChromaticAberration\0GaussionBlur";

			static int select = 0;

			if (ImGui::Combo("Type", &select, items))
			{
				switch (select)
				{
				case 0: mCurrPostProcess = PostProcess::None;					break;
				case 1: mCurrPostProcess = PostProcess::Tint;					break;
				case 2: mCurrPostProcess = PostProcess::GreyNoise;				break;
				case 3: mCurrPostProcess = PostProcess::Burn;					break;
				case 4: mCurrPostProcess = PostProcess::Distort;				break;
				case 5: mCurrPostProcess = PostProcess::Spiral;					break;
				case 6: mCurrPostProcess = PostProcess::HeatHaze;				break;
				case 7: mCurrPostProcess = PostProcess::ChromaticAberration;	break;
				case 8: mCurrPostProcess = PostProcess::GaussionBlur;			break;
				}
			}

			static int selectMode = 0;

			auto modes = "FullScreen\0Area\0Polygon";

			if (ImGui::Combo("Mode", &selectMode, modes))
			{
				switch (selectMode)
				{
				case 0: mCurrPostProcessMode = PostProcessMode::Fullscreen; break;
				case 1: mCurrPostProcessMode = PostProcessMode::Area; break;
				case 2: mCurrPostProcessMode = PostProcessMode::Polygon; break;
				}
			}
		}
	}

	// Run any post-processing steps
	if (mCurrPostProcess != PostProcess::None)
	{
		//These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
		ID3D11ShaderResourceView* const pSRV[1] = { NULL };
		gD3DContext->PSSetShaderResources(0, 1, pSRV);

		if (mCurrPostProcessMode == PostProcessMode::Fullscreen)
		{
			FullScreenPostProcess(mCurrPostProcess);
		}
		else if (mCurrPostProcessMode == PostProcessMode::Area)
		{

			//select an object using ImGui
			static CGameObject* select = nullptr;

			//select the object with ImGui
			if (select == nullptr)
			{
				if (ImGui::Begin("Select Object"))
				{
					for (auto obj : GOM->mObjects)
					{
						if (ImGui::Button(obj->GetName().c_str()))
						{
							select = obj;
						}
					}
				}
				ImGui::End();
			}
			else
			{

				static CVector3 areaPos = { 0,0,0 };
				static CVector2 areaSize = { 0,0 };

				ImGui::DragFloat3("Postion", areaPos.GetValuesArray());
				ImGui::DragFloat2("Size", areaSize.GetValuesArray());

				// Pass a 3D point for the centre of the affected area and the size of the (rectangular) area in world units
				AreaPostProcess(mCurrPostProcess, areaPos, areaSize);

				//display the selected obj name and a button to change it
				auto selectedText = "Selected: " + select->GetName();
				ImGui::Text(selectedText.c_str());
				ImGui::SameLine();
				if (ImGui::Button("Change"))
				{
					select = nullptr;
				}
			}
		}
		else if (mCurrPostProcessMode == PostProcessMode::Polygon)
		{
			static CGameObject* select = nullptr;

			//select the object with ImGui
			if (select == nullptr)
			{
				if (ImGui::Begin("Select Object"))
				{
					for (auto obj : GOM->mObjects)
					{
						if (ImGui::Button(obj->GetName().c_str()))
						{
							select = obj;
						}
					}
				}
				ImGui::End();
			}
			else
			{

				// An array of four points in world space - a tapered square centred at the origin
				static std::array<CVector3, 4> points = { {{-5,5,0}, {-5,-5,0}, {5,5,0}, {5,-5,0} } };

				ImGui::Separator();
				ImGui::Text("Edit Area");
				ImGui::DragFloat3("Top Left", points[0].GetValuesArray());
				ImGui::DragFloat3("Bottom Left", points[1].GetValuesArray());
				ImGui::DragFloat3("Top Right", points[2].GetValuesArray());
				ImGui::DragFloat3("Bottom Right", points[3].GetValuesArray());
				ImGui::Separator();

				static CMatrix4x4 polyMatrix = MatrixTranslation({ select->Position() });

				// Pass an array of 4 points and a matrix. Only supports 4 points.
				PolygonPostProcess(mCurrPostProcess, points, polyMatrix);

				//display the selected obj name and a button to change it
				auto selectedText = "Selected: " + select->GetName();
				ImGui::Text(selectedText.c_str());
				ImGui::SameLine();
				if (ImGui::Button("Change"))
				{
					select = nullptr;
				}
			}
		}


		//These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
		ID3D11ShaderResourceView* nullSRV = nullptr;
		gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

	}
	ImGui::End();
}

// Select the appropriate shader plus any additional textures required for a given post-process
// Helper function shared by full-screen, area and polygon post-processing functions below

void CScene::SelectPostProcessShaderAndTextures(PostProcess postProcess)
{

	//colour

	static float tint[] = { 0.5,0.5,0.5 };

	// Noise scaling adjusts how fine the grey noise is.
	static float grainSize = 140; // Fineness of the noise grain

	static float distorsionLevel = 0.03f; //distorsion level for the dirstorsion effect

	static float heatStrenght = 0.01f;
	static float heatSoftEdge = 0.25f; // Softness of the edge of the circle - range 0.001 (hard edge) to 0.25 (very soft)

	switch (postProcess)
	{
	case CScene::PostProcess::None:
		break;

	case CScene::PostProcess::Copy:

		gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::Tint:

		gD3DContext->PSSetShader(gTintPostProcess, nullptr, 0);


		ImGui::ColorEdit3("Pick colour", tint);

		gPostProcessingConstants.tintColour = tint;

		break;

	case CScene::PostProcess::GreyNoise:

		gD3DContext->PSSetShader(gGreyNoisePostProcess, nullptr, 0);

		// Give pixel shader access to the noise texture
		gD3DContext->PSSetShaderResources(1, 1, &mNoiseMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);

		ImGui::DragFloat("Grain size", &grainSize, 1.0f, 0.0f);

		//set the texture noise scale
		gPostProcessingConstants.noiseScale = { mViewportX / grainSize, mViewportY / grainSize };

		// The noise offset is randomised to give a constantly changing noise effect (like tv static)
		gPostProcessingConstants.noiseOffset = { Random(0.0f, 1.0f), Random(0.0f, 1.0f) };

		//The noise strength (default is 0.5)
		ImGui::DragFloat("Noise Strength", &gPostProcessingConstants.noiseStrength, 0.001f, 0.0f, 1.0f, "%.4f");

		//the distance between the centre of the texture and the beginning of the edge
		ImGui::DragFloat("Edge Distance", &gPostProcessingConstants.noiseEdge, 0.01f, 0.0f, 1.0f, "%.4f");

		break;

	case CScene::PostProcess::Burn:

		gD3DContext->PSSetShader(gBurnPostProcess, nullptr, 0);

		// Give pixel shader access to the burn texture (basically a height map that the burn level ascends)
		gD3DContext->PSSetShaderResources(1, 1, &mBurnMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);

		break;

	case CScene::PostProcess::Distort:

		gD3DContext->PSSetShader(gDistortPostProcess, nullptr, 0);

		// Give pixel shader access to the distortion texture (containts 2D vectors (in R & G) to shift the texture UVs to give a cut-glass impression)
		gD3DContext->PSSetShaderResources(1, 1, &mDistortMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);

		ImGui::DragFloat("Distorsion Level", &distorsionLevel, 0.001f);

		// Set the level of distortion
		gPostProcessingConstants.distortLevel = distorsionLevel;

		break;

	case CScene::PostProcess::Spiral:

		gD3DContext->PSSetShader(gSpiralPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::HeatHaze:

		gD3DContext->PSSetShader(gHeatHazePostProcess, nullptr, 0);

		ImGui::DragFloat("Strength", &heatStrenght, 0.0001f);

		gPostProcessingConstants.heatEffectStrength = heatStrenght;

		ImGui::SliderFloat("Soft Edge", &heatSoftEdge, 0.001f, 0.25f);

		gPostProcessingConstants.heatSoftEdge = heatSoftEdge;

		break;

	case CScene::PostProcess::ChromaticAberration:

		gD3DContext->PSSetShader(gChromaticAberrationPostProcess, nullptr, 0);

		ImGui::DragFloat("Amount", &gPostProcessingConstants.caAmount, 0.0001f,NULL,NULL,"%.5f");
		break;

	case CScene::PostProcess::GaussionBlur:

		gD3DContext->PSSetShader(gGaussionBlurPostProcess, nullptr, 0);
		
		ImGui::DragFloat("Directions",&gPostProcessingConstants.blurDirections,0.1f,0.01f,64.0f);
		ImGui::DragFloat("Quality"	 ,&gPostProcessingConstants.blurQuality,0.1,1.0f,64.0f);
		ImGui::DragFloat("Size"		 ,&gPostProcessingConstants.blurSize,0.1f);


		break;
	}
}

void CScene::FullScreenPostProcess(PostProcess postProcess)
{
	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &mFinalTargetView, mDepthStencilView);

	// Give the pixel shader (post-processing shader) access to the scene texture 
	gD3DContext->PSSetShaderResources(0, 1, &mTextureSRV);
	gD3DContext->PSSetSamplers(0, 1, &gPointSampler); // Use point sampling (no bilinear, trilinear, mip-mapping etc. for most post-processes)


	// Using special vertex shader that creates its own data for a 2D screen quad
	gD3DContext->VSSetShader(g2DQuadVertexShader, nullptr, 0);
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  // Switch off geometry shader when not using it (pass nullptr for first parameter)


	// States - no blending, don't write to depth buffer and ignore back-face culling
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);


	// No need to set vertex/index buffer (see 2D quad vertex shader), just indicate that the quad will be created as a triangle strip
	gD3DContext->IASetInputLayout(NULL); // No vertex data
	gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	// Select shader and textures needed for the required post-processes (helper function above)
	SelectPostProcessShaderAndTextures(postProcess);


	// Set 2D area for full-screen post-processing (coordinates in 0->1 range)
	gPostProcessingConstants.area2DTopLeft = { 0, 0 }; // Top-left of entire screen
	gPostProcessingConstants.area2DSize = { 1, 1 }; // Full size of screen
	gPostProcessingConstants.area2DDepth = 0;        // Depth buffer value for full screen is as close as possible


													 // Pass over the above post-processing settings (also the per-process settings prepared in UpdateScene function below)
	UpdatePostProcessingConstBuffer(gPostProcessingConstBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);


	// Draw a quad
	gD3DContext->Draw(4, 0);
}

// Perform an area post process from "scene texture" to back buffer at a given point in the world, with a given size (world units)

void CScene::AreaPostProcess(PostProcess postProcess, CVector3 worldPoint, CVector2 areaSize)
{
	// First perform a full-screen copy of the scene to back-buffer
	FullScreenPostProcess(PostProcess::Copy);


	// Now perform a post-process of a portion of the scene to the back-buffer (overwriting some of the copy above)
	// Note: The following code relies on many of the settings that were prepared in the FullScreenPostProcess call above, it only
	//       updates a few things that need to be changed for an area process. If you tinker with the code structure you need to be
	//       aware of all the work that the above function did that was also preparation for this post-process area step

	// Select shader/textures needed for required post-process
	SelectPostProcessShaderAndTextures(postProcess);

	// Enable alpha blending - area effects need to fade out at the edges or the hard edge of the area is visible
	// A couple of the shaders have been updated to put the effect into a soft circle
	// Alpha blending isn't enabled for fullscreen and polygon effects so it doesn't affect those (except heat-haze, which works a bit differently)
	gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);


	// Use picking methods to find the 2D position of the 3D point at the centre of the area effect
	auto worldPointTo2D = mCamera->PixelFromWorldPt(worldPoint, mViewportX, mViewportY);
	CVector2 area2DCentre = { worldPointTo2D.x, worldPointTo2D.y };
	float areaDistance = worldPointTo2D.z;

	// Nothing to do if given 3D point is behind the camera
	if (areaDistance < mCamera->NearClip())  return;

	// Convert pixel coordinates to 0->1 coordinates as used by the shader
	area2DCentre.x /= mViewportX;
	area2DCentre.y /= mViewportY;


	// Using new helper function here - it calculates the world space units covered by a pixel at a certain distance from the camera.
	// Use this to find the size of the 2D area we need to cover the world space size requested
	CVector2 pixelSizeAtPoint = mCamera->PixelSizeInWorldSpace(areaDistance, mViewportX, mViewportY);
	CVector2 area2DSize = { areaSize.x / pixelSizeAtPoint.x, areaSize.y / pixelSizeAtPoint.y };

	// Again convert the result in pixels to a result to 0->1 coordinates
	area2DSize.x /= mViewportX;
	area2DSize.y /= mViewportY;


	// Send the area top-left and size into the constant buffer - the 2DQuad vertex shader will use this to create a quad in the right place
	gPostProcessingConstants.area2DTopLeft = area2DCentre - 0.5f * area2DSize; // Top-left of area is centre - half the size
	gPostProcessingConstants.area2DSize = area2DSize;

	// Manually calculate depth buffer value from Z distance to the 3D point and camera near/far clip values. Result is 0->1 depth value
	// We've never seen this full calculation before, it's occasionally useful. It is derived from the material in the Picking lecture
	// Having the depth allows us to have area effects behind normal objects
	gPostProcessingConstants.area2DDepth = mCamera->FarClip() * (areaDistance - mCamera->NearClip()) / (mCamera->FarClip() - mCamera->NearClip());
	gPostProcessingConstants.area2DDepth /= areaDistance;

	// Pass over this post-processing area to shaders (also sends the per-process settings prepared in UpdateScene function below)
	UpdatePostProcessingConstBuffer(gPostProcessingConstBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);


	// Draw a quad
	gD3DContext->Draw(4, 0);
}

// Perform an post process from "scene texture" to back buffer within the given four-point polygon and a world matrix to position/rotate/scale the polygon
void CScene::PolygonPostProcess(PostProcess postProcess, const std::array<CVector3, 4>& points, const CMatrix4x4& worldMatrix)
{
	// First perform a full-screen copy of the scene to back-buffer
	FullScreenPostProcess(PostProcess::Copy);

	// Now perform a post-process of a portion of the scene to the back-buffer (overwriting some of the copy above)
	// Note: The following code relies on many of the settings that were prepared in the FullScreenPostProcess call above, it only
	//       updates a few things that need to be changed for an area process. If you tinker with the code structure you need to be
	//       aware of all the work that the above function did that was also preparation for this post-process area step

	// Select shader/textures needed for required post-process
	SelectPostProcessShaderAndTextures(postProcess);

	// Loop through the given points, transform each to 2D (this is what the vertex shader normally does in most labs)
	for (unsigned int i = 0; i < points.size(); ++i)
	{
		CVector4 modelPosition = CVector4(points[i], 1);
		CVector4 worldPosition = modelPosition * worldMatrix;
		CVector4 viewportPosition = worldPosition * mCamera->ViewProjectionMatrix();

		gPostProcessingConstants.polygon2DPoints[i] = viewportPosition;
	}

	// Pass over the polygon points to the shaders (also sends the per-process settings prepared in UpdateScene function below)
	UpdatePostProcessingConstBuffer(gPostProcessingConstBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);

	// Select the special 2D polygon post-processing vertex shader and draw the polygon
	gD3DContext->VSSetShader(g2DPolygonVertexShader, nullptr, 0);
	gD3DContext->Draw(4, 0);
}

CScene::~CScene()
{
	ReleaseStates();
	if (gPerModelConstantBuffer)			gPerModelConstantBuffer->Release();
	if (gPerFrameConstantBuffer)			gPerFrameConstantBuffer->Release();
	if (gPerFrameLightsConstBuffer)			gPerFrameLightsConstBuffer->Release();
	if (gPerFrameSpotLightsConstBuffer)		gPerFrameSpotLightsConstBuffer->Release();
	if (gPerFrameDirLightsConstBuffer)		gPerFrameDirLightsConstBuffer->Release();
	if (gPerFramePointLightsConstBuffer)	gPerFramePointLightsConstBuffer->Release();
	if (mTargetView)						mTargetView->Release();
	if (mTextrue)							mTextrue->Release();
	if (mTextureSRV)						mTextureSRV->Release();
	if (mDepthStencil)						mDepthStencil->Release();
	if (mDepthStencilView)					mDepthStencilView->Release();

	if (mFinalTargetView)	mFinalTargetView->Release();
	if (mFinalTextrue)		mFinalTextrue->Release();
	if (mFinalTextureSRV)	mFinalTextureSRV->Release();

	ReleaseDefaultShaders();
}

void CScene::Resize(UINT newX, UINT newY)
{
	//set aspect ratio with the new window size
	//broken
	//mCamera->SetAspectRatio(float(newY) / float(newX));

	//set the scene viewport size to the new size
	mViewportX = newX;
	mViewportY = newY;

	//release the textures
	mTextrue->Release();
	mDepthStencil->Release();
	mDepthStencilView->Release();
	mTargetView->Release();
	mTextureSRV->Release();

	// We create a new texture for the scene with new size
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = newX; // Size of the "screen"
	textureDesc.Height = newY;
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


	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;


	// We created the scene texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(gD3DDevice->CreateRenderTargetView(mTextrue, &rtvDesc, &mTargetView)))
	{
		gLastError = "Error creating scene render target view";
	}

	if (FAILED(gD3DDevice->CreateRenderTargetView(mFinalTextrue, &rtvDesc, &mFinalTargetView)))
	{
		gLastError = "Error creating scene render target view";
	}

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	if (FAILED(gD3DDevice->CreateShaderResourceView(mTextrue, &srvDesc, &mTextureSRV)))
	{
		throw std::runtime_error("Error creating scene texture shader resource view");
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(mFinalTextrue, &srvDesc, &mFinalTextureSRV)))
	{
		throw std::runtime_error("Error creating scene texture shader resource view");
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
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	//create it
	if (FAILED(gD3DDevice->CreateTexture2D(&dsDesc, NULL, &mDepthStencil)))
	{
		throw std::runtime_error("Error creating depth stencil");
	}

	//create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(gD3DDevice->CreateDepthStencilView(mDepthStencil, &dsvDesc, &mDepthStencilView)))
	{
		throw std::runtime_error("Error creating depth stencil view ");
	}
}
