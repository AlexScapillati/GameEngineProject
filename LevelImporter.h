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

	bool LoadScene(const std::string& level, CScene* scene);

	void SaveScene(std::string& fileName, CScene* ptrScene);



private:
	
	void ParsePostProcessingEffects(tinyxml2::XMLElement* curr, CScene* scene);
	
	void SavePostProcessingEffect(tinyxml2::XMLElement* curr, CScene* scene);

	void SaveObjects(tinyxml2::XMLElement* el, CScene* ptrScene);

	void SaveVector3(CVector3 v, tinyxml2::XMLElement* el);

	bool ParseScene(tinyxml2::XMLElement* sceneEl, CScene* scene);

	void LoadObject(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadPointLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadSky(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadCamera(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadPlant(tinyxml2::XMLElement* currEntity, CScene* scene);

	bool ParseEntities(tinyxml2::XMLElement* entitiesEl, CScene* scene);
};
