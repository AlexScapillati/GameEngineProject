#pragma once

#include "..\External/tinyxml2/tinyxml2.h"
#include <string>
#include "DX11Scene.h"
#include "Objects/GameObject.h"

class CVector3;

class CLevelImporter
{
public:

	//--------------------------------------------------------------------------------------
	// Scene Parser
	//--------------------------------------------------------------------------------------

	bool LoadScene(const std::string& level, CDX11Scene* scene);

	void SaveScene(std::string& fileName, CDX11Scene* ptrScene);

	CLevelImporter(CDX11Engine* engine);

private:
	
	CDX11Engine* mEngine;

	void ParsePostProcessingEffects(tinyxml2::XMLElement* curr, CDX11Scene* scene);
	
	void SavePostProcessingEffect(tinyxml2::XMLElement* curr, CDX11Scene* scene);

	void SavePositionRotationScale(tinyxml2::XMLElement* obj, CDX11GameObject* it);

	void SaveObjects(tinyxml2::XMLElement* el, CDX11Scene* ptrScene);

	void SaveVector3(CVector3 v, tinyxml2::XMLElement* el);

	bool ParseScene(tinyxml2::XMLElement* sceneEl, CDX11Scene* scene);

	void LoadObject(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	void LoadPointLight(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	void LoadLight(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	void LoadSpotLight(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	void LoadDirLight(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	void LoadSky(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	void LoadCamera(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	void LoadPlant(tinyxml2::XMLElement* currEntity, CDX11Scene* scene);

	bool ParseEntities(tinyxml2::XMLElement* entitiesEl, CDX11Scene* scene);
};
