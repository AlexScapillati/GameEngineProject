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

	//--------------------------------------------------------------------------------------
	// Scene Geometry and Layout
	//--------------------------------------------------------------------------------------

public:


	CScene(std::string fileName)
	{
		mCamera = nullptr;
		mTextrue = nullptr;
		mTargetView = nullptr;
		mTextureSRV = nullptr;

		mObjManager = new CGameObjectManager();

		GOM = mObjManager;

		try
		{
			InitScene(std::move(fileName));
		}
		catch (std::exception e)
		{
			throw std::runtime_error(e.what());
		}

		gCameraOrbitRadius = 60.0f;
		gCameraOrbitSpeed = 1.2f;
		gAmbientColour = { 0.3f, 0.3f, 0.4f };
		gSpecularPower = 256; // Specular power //will be removed since it will be dependent on the material
		lockFPS = true;
		gBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };
		gLightOrbitRadius = 20.0f;
		gLightOrbitSpeed = 0.7f;



		// We also need a depth buffer to go with our portal
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = gViewportWidth; // Size of the "screen"
		textureDesc.Height = gViewportHeight;
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


		// We created the scene texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
		// we use when rendering to it (see RenderScene function below)
		if (FAILED(gD3DDevice->CreateRenderTargetView(mTextrue, NULL, &mTargetView)))
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
	}

	// Prepare the geometry required for the scene
	// Returns true on success
	bool InitScene(std::string fileName);

	void RenderSceneFromCamera(CCamera* camera) const;

	//--------------------------------------------------------------------------------------
	// Scene Render and Update
	//--------------------------------------------------------------------------------------

	void RenderScene(float frameTime) const;

	// frameTime is the time passed since the last frame
	void UpdateScene(float frameTime);

	bool LoadScene(const std::string& level);

	bool ParseScene(tinyxml2::XMLElement* sceneEl);

	void LoadObject(tinyxml2::XMLElement* entitiesEl) const;

	void LoadPointLight(tinyxml2::XMLElement* currEntity);

	void LoadLight(tinyxml2::XMLElement* currEntity) const;

	void LoadSky(tinyxml2::XMLElement* currEntity) const;

	void LoadCamera(tinyxml2::XMLElement* currEntity);

	void LoadPlant(tinyxml2::XMLElement* currEntity) const;

	bool ParseEntities(tinyxml2::XMLElement* entitiesEl);


	~CScene();

private:

	//--------------------------------------------------------------------------------------
	// Scene Data
	//--------------------------------------------------------------------------------------

	CGameObjectManager* mObjManager;

	// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
	bool lockFPS = true;


	// Variables controlling light1's orbiting of the particle emitter
	float gCameraOrbitRadius;
	float gCameraOrbitSpeed;


	// Additional light information
	CVector3 gAmbientColour; // Background level of light (slightly bluish to match the far background, which is dark blue)
	float    gSpecularPower; // Specular power controls shininess - same for all models in this app

	ColourRGBA gBackgroundColor;

	// Variables controlling light1's orbiting of the cube
	float gLightOrbitRadius;
	float gLightOrbitSpeed;

	std::string mDefaultVs;
	std::string mDefaultPs;

	CCamera* mCamera; //WIP

	ID3D11Texture2D* mTextrue;
	ID3D11ShaderResourceView* mTextureSRV;
	ID3D11RenderTargetView* mTargetView;

};