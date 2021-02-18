//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#pragma once

#include "Scene.h"
#include "Plant.h"
#include "Sky.h"
#include <dxgidebug.h>
#include <utility>

#include "SpotLight.h"
#include "DirLight.h"
#include "CPointLight.h"

#include "External\imgui\imgui.h"
#include "External\imgui\imgui_impl_dx11.h"
#include "External\imgui\imgui_impl_win32.h"


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


void SetupGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Setup Platform/Renderer bindings
	ImGui_ImplDX11_Init(gD3DDevice, gD3DContext);
	ImGui_ImplWin32_Init(gHWnd);
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}

CGameObject* selectedObj = nullptr;

void DisplayObjects(CGameObjectManager* GOM)
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

	//if there is an object selected
	if (selectedObj)
	{

		ImGui::Checkbox("Enabled", selectedObj->Enabled());

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

	}
}

void DisplayShadowMaps(CGameObjectManager* GOM)
{

	ImGui::Begin("ShadowMaps");

	for (auto tx : GOM->mShadowsMaps)
	{
		ImGui::NewLine();

		ImTextureID texId = tx;

		ImGui::Image((void*)texId, { 256, 256 });
	}

	ImGui::End();

}


void RenderGui(CGameObjectManager* GOM)
{

	ImGui::Begin("Objects");

	DisplayObjects(GOM);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

void ShutdownGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool CScene::InitScene(std::string fileName)
{
	////--------------- Load meshes ---------------////

	// Load mesh geometry data

	try
	{
		LoadDefaultShaders();
		LoadScene(std::move(fileName));
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

	//SetupGui
	SetupGui();

	return true;
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

	//if the shadowmaps array is not empty
	if (!mObjManager->mShadowsMaps.empty())
	{
		//send the shadow maps to the shaders (slot 5)
		gD3DContext->PSSetShaderResources(5, mObjManager->mShadowsMaps.size(), &mObjManager->mShadowsMaps[0]);
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
void CScene::RenderScene(float frameTime) const
{

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

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
	gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

	// Clear the back buffer to a fixed colour and the depth buffer to the far distance
	gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
	gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Setup the viewport to the size of the main window
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(gViewportWidth);
	vp.Height = static_cast<FLOAT>(gViewportHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gD3DContext->RSSetViewports(1, &vp);

	// Render the scene from the main camera
	RenderSceneFromCamera(mCamera);

	//Render the GUI
	RenderGui(mObjManager);


	////--------------- Scene completion ---------------////

	// When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
	// Set first parameter to 1 to lock to vsync
	if (gSwapChain->Present(lockFPS ? 1 : 0, 0) == DXGI_ERROR_DEVICE_REMOVED)
	{
		gD3DDevice->GetDeviceRemovedReason();

		throw std::runtime_error("Device Removed");
	}
}
//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void CScene::UpdateScene(float frameTime)
{

	static auto rotating = 0.0f;

	auto offset = 10.0f;

	//mObjManager->mSpotLights[0]->SetPosition(CVector3(cos(rotating) * offset, 10.0f, sin(rotating) * offset));

	//mObjManager->mSpotLights[0]->SetRotation({ 0.0f,rotating,0.0f });

	rotating += frameTime;


	mCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);


	// Toggle FPS limiting
	if (KeyHit(Key_P))
	{
		lockFPS = !lockFPS;
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


//--------------------------------------------------------------------------------------
// Scene Parser
//--------------------------------------------------------------------------------------


bool CScene::LoadScene(const std::string& level)
{
	tinyxml2::XMLDocument doc;

	if (doc.LoadFile(level.c_str()) != tinyxml2::XMLError::XML_SUCCESS)
	{
		throw std::runtime_error("Error opening file");
	}

	auto element = doc.FirstChildElement();

	while (element != nullptr)
	{
		std::string elementName = element->Name();
		if (elementName == "Scene")
		{
			try
			{
				ParseScene(element);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}

		element = element->NextSiblingElement();

	}

	return true;
}

bool CScene::ParseScene(tinyxml2::XMLElement* sceneEl)
{
	auto element = sceneEl->FirstChildElement();

	while (element != nullptr)
	{
		std::string elementName = element->Name();

		if (elementName == "Entities")
		{
			try
			{
				ParseEntities(element);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}
		else if (elementName == "Default")
		{
			const auto elementShaders = element->FirstChildElement("Shaders");
			if (elementShaders)
			{
				auto child = elementShaders->FindAttribute("VS");
				if (child) mDefaultVs = child->Value();

				child = elementShaders->FindAttribute("PS");
				if (child) mDefaultPs = child->Value();
			}
			else
			{
				throw std::runtime_error("Error loading default scene values");
			}
		}
		element = element->NextSiblingElement();
	}
	return true;
}

void CScene::LoadObject(tinyxml2::XMLElement* currEntity) const
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = mDefaultVs;
	auto pixelShader = mDefaultPs;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr)
		name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");

	if (geometry)
	{
		const auto idAttr = geometry->FindAttribute("ID");
		if (idAttr) ID = idAttr->Value();

		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();

		const auto VsAttr = geometry->FindAttribute("VS");
		if (VsAttr) vertexShader = VsAttr->Value();

		const auto PsAttr = geometry->FindAttribute("PS");
		if (PsAttr) pixelShader = PsAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = { positionEl->FindAttribute("X")->FloatValue(),
				positionEl->FindAttribute("Y")->FloatValue(),
				positionEl->FindAttribute("Z")->FloatValue() };
	}


	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = { ToRadians(rotationEl->FindAttribute("X")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Y")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Z")->FloatValue()) };
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	try
	{
		if (ID.empty())
		{
			auto obj = new CGameObject(mesh, name, diffuse, vertexShader, pixelShader, pos, rot, scale);

			mObjManager->AddObject(obj);
		}
		else
		{
			auto obj = new CGameObject(ID, name, vertexShader, pixelShader, pos, rot, scale);
			mObjManager->AddObject(obj);
		}

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void CScene::LoadPointLight(tinyxml2::XMLElement* currEntity)
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = mDefaultVs;
	auto pixelShader = mDefaultPs;

	CVector3 colour = { 0,0,0 };
	float strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	CVector3 facing = { 0,0,0 };
	auto scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr) name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");
	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();

		const auto VsAttr = geometry->FindAttribute("VS");
		if (VsAttr) vertexShader = VsAttr->Value();

		const auto PsAttr = geometry->FindAttribute("PS");
		if (PsAttr) pixelShader = PsAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = { positionEl->FindAttribute("X")->FloatValue(),
				positionEl->FindAttribute("Y")->FloatValue(),
				positionEl->FindAttribute("Z")->FloatValue() };
	}


	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = { ToRadians(rotationEl->FindAttribute("X")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Y")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Z")->FloatValue()) };
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue() * strength;
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = { colourEl->FindAttribute("X")->FloatValue(),
					colourEl->FindAttribute("Y")->FloatValue(),
					colourEl->FindAttribute("Z")->FloatValue() };
	}


	try
	{
		auto obj = new CPointLight(mesh, name, diffuse, vertexShader, pixelShader, colour, strength, pos, rot, scale);

		mObjManager->AddPointLight(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void CScene::LoadLight(tinyxml2::XMLElement* currEntity) const
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = mDefaultVs;
	auto pixelShader = mDefaultPs;

	CVector3 colour = { 0,0,0 };
	float strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	CVector3 facing = { 0,0,0 };
	auto scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr) name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");
	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();

		const auto VsAttr = geometry->FindAttribute("VS");
		if (VsAttr) vertexShader = VsAttr->Value();

		const auto PsAttr = geometry->FindAttribute("PS");
		if (PsAttr) pixelShader = PsAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = { positionEl->FindAttribute("X")->FloatValue(),
				positionEl->FindAttribute("Y")->FloatValue(),
				positionEl->FindAttribute("Z")->FloatValue() };
	}


	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = { ToRadians(rotationEl->FindAttribute("X")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Y")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Z")->FloatValue()) };
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue() * strength;
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = { colourEl->FindAttribute("X")->FloatValue(),
					colourEl->FindAttribute("Y")->FloatValue(),
					colourEl->FindAttribute("Z")->FloatValue() };
	}

	const auto facingEl = currEntity->FirstChildElement("Facing");

	if (facingEl)
	{
		facing = { facingEl->FindAttribute("X")->FloatValue(),
					facingEl->FindAttribute("Y")->FloatValue(),
					facingEl->FindAttribute("Z")->FloatValue() };

		facing = Normalise(facing);

		//the light is a spot light so create spotLight obj
		try
		{
			//if there is position create a spotlight
			if (positionEl)
			{
				auto obj = new CSpotLight(mesh, name, diffuse, vertexShader, pixelShader, colour, strength, pos, rot, scale, facing);

				mObjManager->AddSpotLight(obj);
			}
			else
			{
				//otherwise create a directional light
				auto obj = new CDirLight(mesh, name, diffuse, vertexShader, pixelShader, colour, strength, pos, rot, scale, facing);

				mObjManager->AddDirLight(obj);
			}

		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(std::string(e.what()) + " of object " + name);
		}
	}
	else
	{
		try
		{
			auto obj = new CLight(mesh, name, diffuse, vertexShader, pixelShader, colour, strength, pos, rot, scale);

			mObjManager->AddLight(obj);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(std::string(e.what()) + " of object " + name);
		}
	}
}

void CScene::LoadSky(tinyxml2::XMLElement* currEntity) const
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = mDefaultVs;
	auto pixelShader = mDefaultPs;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr)
		name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");

	if (geometry)
	{
		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();

		const auto VsAttr = geometry->FindAttribute("VS");
		if (VsAttr) vertexShader = VsAttr->Value();

		const auto PsAttr = geometry->FindAttribute("PS");
		if (PsAttr) pixelShader = PsAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = { positionEl->FindAttribute("X")->FloatValue(),
				positionEl->FindAttribute("Y")->FloatValue(),
				positionEl->FindAttribute("Z")->FloatValue() };
	}


	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = { ToRadians(rotationEl->FindAttribute("X")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Y")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Z")->FloatValue()) };
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	try
	{
		auto obj = new CSky(mesh, name, diffuse, vertexShader, pixelShader, pos, rot, scale);

		mObjManager->AddObject(obj);

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void CScene::LoadCamera(tinyxml2::XMLElement* currEntity)
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = mDefaultVs;
	auto pixelShader = mDefaultPs;
	const auto FOV = PI / 3;
	const auto aspectRatio = 1.333333373f;
	const auto nearClip = 0.100000015f;
	const auto farClip = 10000.0f;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr) name = entityNameAttr->Value();


	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = { positionEl->FindAttribute("X")->FloatValue(),
				positionEl->FindAttribute("Y")->FloatValue(),
				positionEl->FindAttribute("Z")->FloatValue() };
	}


	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = { ToRadians(rotationEl->FindAttribute("X")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Y")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Z")->FloatValue()) };
	}

	mCamera = new CCamera(pos, rot, FOV, aspectRatio, nearClip, farClip);

	if (!mCamera)
	{
		throw std::runtime_error("Error initializing camera");
	}
}

void CScene::LoadPlant(tinyxml2::XMLElement* currEntity) const
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = mDefaultVs;
	auto pixelShader = mDefaultPs;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
	auto scale = 1.0f;

	const auto entityNameAttr = currEntity->FindAttribute("Name");
	if (entityNameAttr)
		name = entityNameAttr->Value();

	const auto geometry = currEntity->FirstChildElement("Geometry");

	if (geometry)
	{
		const auto idAttr = geometry->FindAttribute("ID");
		if (idAttr) ID = idAttr->Value();

		const auto meshAttr = geometry->FindAttribute("Mesh");
		if (meshAttr) mesh = meshAttr->Value();

		const auto diffuseAttr = geometry->FindAttribute("Diffuse");
		if (diffuseAttr) diffuse = diffuseAttr->Value();

		const auto VsAttr = geometry->FindAttribute("VS");
		if (VsAttr) vertexShader = VsAttr->Value();

		const auto PsAttr = geometry->FindAttribute("PS");
		if (PsAttr) pixelShader = PsAttr->Value();
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = { positionEl->FindAttribute("X")->FloatValue(),
				positionEl->FindAttribute("Y")->FloatValue(),
				positionEl->FindAttribute("Z")->FloatValue() };
	}


	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = { ToRadians(rotationEl->FindAttribute("X")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Y")->FloatValue()),
				ToRadians(rotationEl->FindAttribute("Z")->FloatValue()) };
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	try
	{
		if (ID.empty())
		{

			auto obj = new CPlant(mesh, name, diffuse, vertexShader, pixelShader, pos, rot, scale);
			mObjManager->AddObject(obj);
		}
		else
		{
			auto obj = new CPlant(ID, name, vertexShader, pixelShader, pos, rot, scale);
			mObjManager->AddObject(obj);
		}

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

bool CScene::ParseEntities(tinyxml2::XMLElement* entitiesEl)
{

	auto currEntity = entitiesEl->FirstChildElement();

	while (currEntity)
	{
		std::string entityName = currEntity->Name();

		if (entityName == "Entity")
		{
			const auto type = currEntity->FindAttribute("Type");

			if (type)
			{
				std::string typeValue = type->Value();

				if (typeValue == "GameObject")
				{
					LoadObject(currEntity);
				}
				else if (typeValue == "Light")
				{
					LoadLight(currEntity);
				}
				else if (typeValue == "PointLight")
				{
					LoadPointLight(currEntity);
				}
				else if (typeValue == "Sky")
				{
					LoadSky(currEntity);
				}
				else if (typeValue == "Plant")
				{
					LoadPlant(currEntity);
				}
				else if (typeValue == "Camera")
				{
					LoadCamera(currEntity);
				}
			}
		}
		currEntity = currEntity->NextSiblingElement();
	}
	return true;
}

CScene::~CScene()
{

	ShutdownGui();

	ReleaseStates();
	if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
	if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();
	if (gPerFrameLightsConstBuffer) gPerFrameLightsConstBuffer->Release();
	if (gPerFrameSpotLightsConstBuffer) gPerFrameSpotLightsConstBuffer->Release();
	if (gPerFrameDirLightsConstBuffer) gPerFrameDirLightsConstBuffer->Release();
	if (gPerFramePointLightsConstBuffer) gPerFramePointLightsConstBuffer->Release();

	ReleaseDefaultShaders();

	delete mCamera;

	delete mObjManager;
}
