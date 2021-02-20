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

	void RenderSceneFromCamera(CCamera* camera) const;

	//--------------------------------------------------------------------------------------
	// Scene Render and Update
	//--------------------------------------------------------------------------------------

	void RenderScene(float frameTime) const;

	// frameTime is the time passed since the last frame
	void UpdateScene(float frameTime);

	std::string mDefaultVs;
	std::string mDefaultPs;

	std::unique_ptr<CGameObjectManager> mObjManager;

	std::unique_ptr<CCamera> mCamera;


	~CScene();

private:

	//--------------------------------------------------------------------------------------
	// Scene Data
	//--------------------------------------------------------------------------------------

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


	ID3D11Texture2D* mTextrue;
	ID3D11ShaderResourceView* mTextureSRV;
	ID3D11RenderTargetView* mTargetView;

};