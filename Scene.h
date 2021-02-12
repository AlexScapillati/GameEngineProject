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

		mObjManager = new CGameObjectManager();

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
		gSpecularPower = 256; // Specular powe
		lockFPS = true;
		gBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };
		gLightOrbitRadius = 20.0f;
		gLightOrbitSpeed = 0.7f;
		
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

};