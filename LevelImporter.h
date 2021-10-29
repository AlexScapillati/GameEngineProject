#pragma once

#include "tinyxml2\tinyxml2.h"
#include <string>
#include "Scene.h"

class CLevelImporter
{
public:

	//--------------------------------------------------------------------------------------
	// Scene Parser
	//--------------------------------------------------------------------------------------

	bool LoadScene(const std::string& level, IScene* scene);

	void SaveScene(std::string& fileName, IScene* ptrScene);

	CLevelImporter(CDX11Engine* engine);

private:
	
	CDX11Engine* mEngine;

	void ParsePostProcessingEffects(tinyxml2::XMLElement* curr, IScene* scene);
	
	void SavePostProcessingEffect(tinyxml2::XMLElement* curr, IScene* scene);

	void SavePositionRotationScale(tinyxml2::XMLElement* obj, CGameObject* it);

	void SaveObjects(tinyxml2::XMLElement* el, IScene* ptrScene);

	void SaveVector3(CVector3 v, tinyxml2::XMLElement* el);

	bool ParseScene(tinyxml2::XMLElement* sceneEl, IScene* scene);

	void LoadObject(tinyxml2::XMLElement* currEntity, IScene* scene);

	void LoadPointLight(tinyxml2::XMLElement* currEntity, IScene* scene);

	void LoadLight(tinyxml2::XMLElement* currEntity, IScene* scene);

	void LoadSpotLight(tinyxml2::XMLElement* currEntity, IScene* scene);

	void LoadDirLight(tinyxml2::XMLElement* currEntity, IScene* scene);

	void LoadSky(tinyxml2::XMLElement* currEntity, IScene* scene);

	void LoadCamera(tinyxml2::XMLElement* currEntity, IScene* scene);

	void LoadPlant(tinyxml2::XMLElement* currEntity, IScene* scene);

	bool ParseEntities(tinyxml2::XMLElement* entitiesEl, IScene* scene);
};
