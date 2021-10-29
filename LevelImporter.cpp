
#pragma once

#include "LevelImporter.h"

#include "GameObject.h"
#include "Light.h"
#include "DirLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "Sky.h"
#include "Camera.h"
#include "Plant.h"

bool CLevelImporter::LoadScene(const std::string& level, IScene* scene)
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
				ParseScene(element, scene);
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

void CLevelImporter::SaveScene(std::string& fileName /* ="" */, IScene* ptrScene)
{
	if (fileName.empty())
	{
		//save the scene in the project folder
		return;
	}

	tinyxml2::XMLDocument doc;

	const auto scene = doc.NewElement("Scene");

	doc.InsertFirstChild(scene);

	const auto def = scene->InsertNewChildElement("Default");

	auto defShaders = def->InsertNewChildElement("Shaders");

	const auto entities = scene->InsertNewChildElement("Entities");

	SaveObjects(entities, ptrScene);


	const auto ppEffects = scene->InsertNewChildElement("PostProcessingEffects");

	SavePostProcessingEffect(ppEffects, ptrScene);

	doc.InsertEndChild(scene);

	if (doc.SaveFile(fileName.c_str()) != tinyxml2::XMLError::XML_SUCCESS)
	{
		throw std::runtime_error("unable to save");
	}
}

CLevelImporter::CLevelImporter(CDX11Engine* engine)
{
	mEngine = engine;
}


void CLevelImporter::SavePositionRotationScale(tinyxml2::XMLElement* obj, CGameObject* it)
{
	//save position, position and scale
	auto childEl = obj->InsertNewChildElement("Position");
	SaveVector3(it->Position(), childEl);
	childEl = obj->InsertNewChildElement("Rotation");
	SaveVector3(ToDegrees(it->Rotation()), childEl);
	childEl = obj->InsertNewChildElement("Scale");
	SaveVector3(it->Scale(), childEl);
}

void CLevelImporter::SaveObjects(tinyxml2::XMLElement* el, IScene* ptrScene)
{
	//----------------------------------------------------
	//	Game Objects
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mObjects)
	{
		const auto obj = el->InsertNewChildElement("Entity");

		if (auto sky = dynamic_cast<CSky*>(it))
		{
			obj->SetAttribute("Type", "Sky");
		}
		else if (auto plant = dynamic_cast<CPlant*>(it))
		{
			obj->SetAttribute("Type", "Plant");
		}
		else
		{
			obj->SetAttribute("Type", "GameObject");
		}

		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		if (it->Material()->HasNormals())
		{
			std::string id = it->Mesh()->MeshFileName();

			childEl->SetAttribute("ID", id.c_str());
		}
		else
		{
			childEl->SetAttribute("Mesh", it->Mesh()->MeshFileName().c_str());
			childEl->SetAttribute("Diffuse", it->TextrueFileName().c_str());
		}

		SavePositionRotationScale(obj, it);

		// Ambient Map
		if (it->AmbientMapEnabled())
		{
			childEl = obj->InsertNewChildElement("AmbientMap");
			childEl->SetAttribute("Enabled", it->AmbientMapEnabled());
			childEl->SetAttribute("Size", (int)it->AmbientMap()->Size());
		}
	}

	//----------------------------------------------------
	//	Simple Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "Light");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->Mesh()->MeshFileName().c_str());
		childEl->SetAttribute("Diffuse", it->TextrueFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Spot Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mSpotLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "SpotLight");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->Mesh()->MeshFileName().c_str());
		childEl->SetAttribute("Diffuse", it->TextrueFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());

	}

	//----------------------------------------------------
	//	Directional Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mDirLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "DirectionalLight");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->Mesh()->MeshFileName().c_str());
		childEl->SetAttribute("Diffuse", it->TextrueFileName().c_str());

		//save rotation and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());

	}

	//----------------------------------------------------
	//	Point Lights
	//----------------------------------------------------

	for (const auto it : ptrScene->GetObjectManager()->mPointLights)
	{
		const auto obj = el->InsertNewChildElement("Entity");
		obj->SetAttribute("Type", "PointLight");
		obj->SetAttribute("Name", it->Name().c_str());

		auto childEl = obj->InsertNewChildElement("Geometry");

		childEl->SetAttribute("Mesh", it->Mesh()->MeshFileName().c_str());
		childEl->SetAttribute("Diffuse", it->TextrueFileName().c_str());

		//save position, position and scale
		SavePositionRotationScale(obj, it);

		//save colour and strength
		childEl = obj->InsertNewChildElement("Colour");
		SaveVector3(it->GetColour(), childEl);
		childEl = obj->InsertNewChildElement("Strength");
		childEl->SetAttribute("S", it->GetStrength());
	}

	//----------------------------------------------------
	//	Camera
	//----------------------------------------------------

	const auto camera = ptrScene->mCamera.get();

	const auto obj = el->InsertNewChildElement("Entity");
	obj->SetAttribute("Type", "Camera");

	//save position, position
	auto childEl = obj->InsertNewChildElement("Position");
	SaveVector3(camera->Position(), childEl);
	childEl = obj->InsertNewChildElement("Rotation");
	SaveVector3(ToDegrees(camera->Rotation()), childEl);
}

void CLevelImporter::SaveVector3(CVector3 v, tinyxml2::XMLElement* el)
{
	el->SetAttribute("X", v.x);
	el->SetAttribute("Y", v.y);
	el->SetAttribute("Z", v.z);
}

bool CLevelImporter::ParseScene(tinyxml2::XMLElement* sceneEl, IScene* scene)
{
	auto element = sceneEl->FirstChildElement();

	while (element != nullptr)
	{
		std::string elementName = element->Name();

		if (elementName == "Entities")
		{
			try
			{
				ParseEntities(element, scene);
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
				if (child) scene->mDefaultVs = child->Value();

				child = elementShaders->FindAttribute("PS");
				if (child) scene->mDefaultPs = child->Value();
			}
			else
			{
				throw std::runtime_error("Error loading default scene values");
			}
		}
		else if (elementName == "PostProcessingEffects")
		{
			ParsePostProcessingEffects(element, scene);
		}
		element = element->NextSiblingElement();
	}

	return true;
}

CVector3 LoadVector3(tinyxml2::XMLElement* el)
{
	return { float(el->FindAttribute("X")->DoubleValue()),
			float(el->FindAttribute("Y")->DoubleValue()),
			float(el->FindAttribute("Z")->DoubleValue()) };
}

void CLevelImporter::LoadObject(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;

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
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto ambientMapEl = currEntity->FirstChildElement("AmbientMap");

	bool enabled = false;
	int size = 1;

	if (ambientMapEl)
	{
		enabled = ambientMapEl->FindAttribute("Enabled")->BoolValue();
		size = ambientMapEl->FindAttribute("Size")->IntValue();
	}

	// Create objects
	CGameObject* obj;

	try
	{
		if (ID.empty())
		{
			obj = new CGameObject(mEngine, mesh, name, diffuse, pos, rot, scale);

		}
		else
		{
			obj = new CGameObject(mEngine, ID, name, pos, rot, scale);
		}
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}

	// Set ambient map values
	obj->AmbientMapEnabled() = enabled;
	obj->AmbientMap()->SetSize(size);

	// Add it to the object manager
	scene->mObjManager->AddObject(obj);
}

void CLevelImporter::LoadPointLight(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
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
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}

	try
	{
		const auto obj = new CPointLight(mEngine, mesh, name, diffuse, colour, strength, pos, rot, scale);
		scene->mObjManager->AddPointLight(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}

}

void CLevelImporter::LoadLight(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
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

	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = LoadVector3(rotationEl);
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}
	try
	{
		const auto obj = new CLight(mEngine, mesh, name, diffuse, colour, strength, pos, rot, scale);

		scene->mObjManager->AddLight(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}


void CLevelImporter::LoadSpotLight(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
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

	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = LoadVector3(rotationEl);
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}

	try
	{
		const auto obj = new CSpotLight(mEngine, mesh, name, diffuse, colour, strength, pos, rot, scale);

		scene->mObjManager->AddSpotLight(obj);

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}


void CLevelImporter::LoadDirLight(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

	CVector3 colour = { 0,0,0 };
	float strength = 0;
	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };
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

	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = LoadVector3(rotationEl);
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	const auto strengthEl = currEntity->FirstChildElement("Strength");
	if (strengthEl)
	{
		strength = strengthEl->FindAttribute("S")->FloatValue();
	}

	const auto colourEl = currEntity->FirstChildElement("Colour");
	if (colourEl)
	{
		colour = LoadVector3(colourEl);
	}

	try
	{
		const auto obj = new CDirLight(mEngine, mesh, name, diffuse, colour, strength, pos, rot, scale);

		scene->mObjManager->AddDirLight(obj);

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}


void CLevelImporter::LoadSky(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;

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
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	// No ambient map for the sky object

	try
	{
		const auto obj = new CSky(mEngine, mesh, name, diffuse, pos, rot, scale);

		scene->mObjManager->AddObject(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void CLevelImporter::LoadCamera(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string mesh;
	std::string diffuse;
	auto vertexShader = scene->mDefaultVs;
	auto pixelShader = scene->mDefaultPs;
	const auto FOV = PI / 3;
	const auto aspectRatio = 1.333333373f;
	const auto nearClip = 0.100000015f;
	const auto farClip = 10000.0f;

	CVector3 pos = { 0,0,0 };
	CVector3 rot = { 0,0,0 };

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	scene->mCamera = std::make_unique<CCamera>(pos, rot, FOV, aspectRatio, nearClip, farClip);

	if (!scene->mCamera)
	{
		throw std::runtime_error("Error initializing camera");
	}
}

void CLevelImporter::LoadPlant(tinyxml2::XMLElement* currEntity, IScene* scene)
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;

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
	}

	const auto positionEl = currEntity->FirstChildElement("Position");
	if (positionEl)
	{
		pos = LoadVector3(positionEl);
	}

	const auto rotationEl = currEntity->FirstChildElement("Rotation");
	if (rotationEl)
	{
		rot = ToRadians(LoadVector3(rotationEl));
	}

	const auto scaleEl = currEntity->FirstChildElement("Scale");
	if (scaleEl)
	{
		scale = scaleEl->FindAttribute("X")->FloatValue();
	}

	// Ambient Map settings

	const auto ambientMapEl = currEntity->FirstChildElement("AmbientMap");

	bool enabled = false;
	int size = 1;

	if (ambientMapEl)
	{
		enabled = ambientMapEl->FindAttribute("Enabled")->BoolValue();
		size = ambientMapEl->FindAttribute("Size")->IntValue();
	}

	try
	{
		if (ID.empty())
		{
			const auto obj = new CPlant(mEngine, mesh, name, diffuse, pos, rot, scale);

			// Set ambient map values
			obj->AmbientMapEnabled() = enabled;
			obj->AmbientMap()->SetSize(size);

			scene->mObjManager->AddObject(obj);
		}
		else
		{
			const auto obj = new CPlant(mEngine, ID, name, pos, rot, scale);

			// Set ambient map values
			obj->AmbientMapEnabled() = enabled;
			obj->AmbientMap()->SetSize(size);

			scene->mObjManager->AddObject(obj);
		}
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

void CLevelImporter::SavePostProcessingEffect(tinyxml2::XMLElement* curr, IScene* scene)
{
	// Save the Type and mode for every effect
	for (auto pp : scene->mPostProcessingFilters)
	{
		const auto ppEl = curr->InsertNewChildElement("Effect");

		// Cast the type string with the corresponding enum
		ppEl->SetAttribute("Type", scene->mPostProcessStrings[(int)pp.type].c_str());
		ppEl->SetAttribute("Mode", scene->mPostProcessModeStrings[(int)pp.mode].c_str());
	}

	// Save settings
	// Create an array of floats
	float settings[sizeof(PostProcessingConstants) / sizeof(float)];

	// Copy the postprocessing constants struct in the array of floats with memcpy
	memcpy(settings, &gPostProcessingConstants, sizeof(PostProcessingConstants));

	// Insert the settings element
	const auto settingsEl = curr->InsertNewChildElement("Settings");

	// For every setting 
	for (unsigned long long int i = 0; i < ARRAYSIZE(settings); ++i)
	{
		// create a different name for each setting
		std::string name = "setting";
		name.append(std::to_string(i));

		// set the attribute 
		settingsEl->SetAttribute(name.c_str(), settings[i]);
	}
}

void CLevelImporter::ParsePostProcessingEffects(tinyxml2::XMLElement* curr, IScene* scene)
{
	auto currEffect = curr->FirstChildElement();

	while (currEffect)
	{
		std::string item = currEffect->Name();

		if (item == "Effect")
		{
			std::string typeValue;
			std::string modeValue;

			const auto type = currEffect->FindAttribute("Type");
			if (type)
			{
				typeValue = type->Value();
			}

			const auto mode = currEffect->FindAttribute("Mode");
			if (mode)
				modeValue = mode->Value();

			IScene::PostProcessFilter filter;

			for (unsigned long long int i = 0; i < ARRAYSIZE(scene->mPostProcessModeStrings); ++i)
			{
				if (modeValue._Equal(scene->mPostProcessModeStrings[i]))
				{
					filter.mode = (IScene::PostProcessMode)i;
				}
			}

			for (int i = 0; i < ARRAYSIZE(scene->mPostProcessStrings); ++i)
			{
				if (typeValue._Equal(scene->mPostProcessStrings[i]))
				{
					filter.type = (IScene::PostProcess)i;
				}
			}

			scene->mPostProcessingFilters.push_back(filter);

		}
		// After Loading all the effects
		// Load the settings
		else if (item == "Settings")
		{
			float values[sizeof(gPostProcessingConstants) / sizeof(float)];

			for (unsigned long long int i = 0; i < sizeof(gPostProcessingConstants) / sizeof(float); ++i)
			{

				// get the different name for each setting
				std::string name = "setting";
				name.append(std::to_string(i));

				const auto currSetting = currEffect->FindAttribute(name.c_str());

				values[i] = currSetting->FloatValue();
			}

			memcpy(&gPostProcessingConstants, values, sizeof(values));

		}
		currEffect = currEffect->NextSiblingElement();
	}

}

bool CLevelImporter::ParseEntities(tinyxml2::XMLElement* entitiesEl, IScene* scene)
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
					LoadObject(currEntity, scene);
				}
				else if (typeValue == "Light")
				{
					LoadLight(currEntity, scene);
				}
				else if (typeValue == "PointLight")
				{
					LoadPointLight(currEntity, scene);
				}
				else if (typeValue == "DirectionalLight")
				{
					LoadDirLight(currEntity, scene);
				}
				else if (typeValue == "SpotLight")
				{
					LoadSpotLight(currEntity, scene);
				}
				else if (typeValue == "Sky")
				{
					LoadSky(currEntity, scene);
				}
				else if (typeValue == "Plant")
				{
					LoadPlant(currEntity, scene);
				}
				else if (typeValue == "Camera")
				{
					LoadCamera(currEntity, scene);
				}
			}
		}
		currEntity = currEntity->NextSiblingElement();
	}
	return true;
}
