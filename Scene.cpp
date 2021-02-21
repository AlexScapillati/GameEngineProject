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

CGameObjectManager* GOM;

CGameObject* selectedObj = nullptr;

void DisplayObjects()
{

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

	if (selectedObj)
	{
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
	}

	//if there is an object selected
	if (selectedObj)
	{

		ImGui::Checkbox("Enabled", selectedObj->Enabled());

		ImGui::Checkbox("Toggle ambient Map", selectedObj->AmbientMapEnabled());

		//display the transform component
		ImGui::NewLine();
		ImGui::Text("Transform");

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
				ImGui::Begin("ColourPicker", &colourPickerOpen);

				auto colour = light->GetColour().GetValuesArray();

				if (ImGui::ColorPicker3("Colour", colour))
				{
					light->SetColour(colour);
				}
				ImGui::End();
			}

			//modify strength
			auto st = light->GetStrength();

			if (ImGui::DragFloat("Strength", &st, 1.0f, 0.0f, D3D11_FLOAT32_MAX))
			{
				light->SetStrength(st);
			}

			//if it is a spotlight let it modify few things
			if (auto spotLight = dynamic_cast<CSpotLight*>(selectedObj))
			{
				//modify facing
				auto facingV = spotLight->GetFacing().GetValuesArray();

				if (ImGui::DragFloat3("Facing", facingV, 0.001f, -1.0f, 1.0f))
				{
					CVector3 facing = Normalise(facingV);
					spotLight->SetFacing(facing);
				}


				//modify cone angle

				auto coneAngle = spotLight->GetConeAngle();

				if (ImGui::DragFloat("Cone Angle", &coneAngle, 1.0f, 0.0f, 180.0f))
				{
					spotLight->SetConeAngle(coneAngle);
				}

				//modify shadow map size

				auto size = spotLight->getShadowMapSize();

				if (ImGui::DragInt("ShadowMapsSize", &size, 1.0f, 2, 16384))
				{
					if (size < 16384)
					{
						spotLight->SetShadowMapsSize(size);
					}
					else
					{
						std::runtime_error("Number Too big!");
					}
				}
			}
			else if (auto dirLight = dynamic_cast<CDirLight*>(selectedObj))
			{
				//modify direction

				auto facingV = dirLight->GetDirection().GetValuesArray();

				if (ImGui::DragFloat3("Facing", facingV, 0.001f, -1.0f, 1.0f))
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
	gLightOrbitRadius = 20.0f;
	gLightOrbitSpeed = 0.7f;



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

	if (!gPerFrameConstantBuffer ||
		!gPerModelConstantBuffer ||
		!gPerFrameDirLightsConstBuffer ||
		!gPerFrameLightsConstBuffer ||
		!gPerFrameSpotLightsConstBuffer ||
		!gPerFramePointLightsConstBuffer)
	{
		throw std::runtime_error("Error creating constant buffers");
	}
}

//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render everything in the scene from the given camera
void CScene::RenderSceneFromCamera(CCamera* camera) const
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
ID3D11ShaderResourceView* CScene::RenderScene(float frameTime) const
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

	// Set the back buffer as the target for rendering and select the main depth buffer.
	// When finished the back buffer is sent to the "front buffer" - which is the monitor.
	gD3DContext->OMSetRenderTargets(1, &mTargetView, mDepthStencilView);

	// Clear the back buffer to a fixed colour and the depth buffer to the far distance
	gD3DContext->ClearRenderTargetView(mTargetView, &gBackgroundColor.r);
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

	return mTextureSRV;
}
//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void CScene::UpdateScene(float frameTime)
{


	mCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);


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


CScene::~CScene()
{

	ReleaseStates();
	if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
	if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();
	if (gPerFrameLightsConstBuffer) gPerFrameLightsConstBuffer->Release();
	if (gPerFrameSpotLightsConstBuffer) gPerFrameSpotLightsConstBuffer->Release();
	if (gPerFrameDirLightsConstBuffer) gPerFrameDirLightsConstBuffer->Release();
	if (gPerFramePointLightsConstBuffer) gPerFramePointLightsConstBuffer->Release();
	if (mTargetView) mTargetView->Release();
	if (mTextrue) mTextrue->Release();
	if (mTextureSRV) mTextureSRV->Release();
	if (mDepthStencil) mDepthStencil->Release();
	if (mDepthStencilView) mDepthStencilView->Release();

	ReleaseDefaultShaders();
}

void CScene::Resize(UINT newX, UINT newY)
{

	mViewportX = newX;
	mViewportY = newY;

	mTextrue->Release();
	mDepthStencil->Release();
	mDepthStencilView->Release();
	mTargetView->Release();
	mTextureSRV->Release();

	// We also need a depth buffer to go with our texture
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
