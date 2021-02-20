#pragma once
#include "tinyxml2\tinyxml2.h"
#include <string>
#include "Scene.h"

class CLevelImporter
{
public:

	CLevelImporter();

	//--------------------------------------------------------------------------------------
	// Scene Parser
	//--------------------------------------------------------------------------------------


	bool LoadScene(const std::string& level, CScene* scene);

	void SaveScene();

	~CLevelImporter();

private:


	bool ParseScene(tinyxml2::XMLElement* sceneEl, CScene* scene);

	void LoadObject(tinyxml2::XMLElement* currEntity, CScene* scene) const;

	void LoadPointLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadLight(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadSky(tinyxml2::XMLElement* currEntity, CScene* scene) const;

	void LoadCamera(tinyxml2::XMLElement* currEntity, CScene* scene);

	void LoadPlant(tinyxml2::XMLElement* currEntity, CScene* scene) const;

	bool ParseEntities(tinyxml2::XMLElement* entitiesEl, CScene* scene);


};

