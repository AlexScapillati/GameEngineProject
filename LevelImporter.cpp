#include "LevelImporter.h"

#include "GameObject.h"
#include "Light.h"
#include "DirLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "Sky.h"
#include "Camera.h"
#include "Plant.h"


 CLevelImporter::CLevelImporter()
{

}

 bool CLevelImporter::LoadScene(const std::string& level, CScene* scene)
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

 void CLevelImporter::SaveScene()
 {
	 throw std::runtime_error("Not implemented yet");
 }

 bool CLevelImporter::ParseScene(tinyxml2::XMLElement* sceneEl, CScene* scene)
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
		element = element->NextSiblingElement();
	}

	return true;
}

 void CLevelImporter::LoadObject(tinyxml2::XMLElement* currEntity, CScene* scene) const
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = scene->mDefaultVs;
	auto pixelShader = scene->mDefaultPs;

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

			scene->mObjManager->AddObject(obj);
		}
		else
		{
			auto obj = new CGameObject(ID, name, vertexShader, pixelShader, pos, rot, scale);
			scene->mObjManager->AddObject(obj);
		}

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

 void CLevelImporter::LoadPointLight(tinyxml2::XMLElement* currEntity, CScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = scene->mDefaultVs;
	auto pixelShader = scene->mDefaultPs;

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

		scene->mObjManager->AddPointLight(obj);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

 void CLevelImporter::LoadLight(tinyxml2::XMLElement* currEntity, CScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = scene->mDefaultVs;
	auto pixelShader = scene->mDefaultPs;

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

				scene->mObjManager->AddSpotLight(obj);
			}
			else
			{
				//otherwise create a directional light
				auto obj = new CDirLight(mesh, name, diffuse, vertexShader, pixelShader, colour, strength, pos, rot, scale, facing);

				scene->mObjManager->AddDirLight(obj);
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

			scene->mObjManager->AddLight(obj);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(std::string(e.what()) + " of object " + name);
		}
	}
}

 void CLevelImporter::LoadSky(tinyxml2::XMLElement* currEntity, CScene* scene) const
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = scene->mDefaultVs;
	auto pixelShader = scene->mDefaultPs;

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

		scene->mObjManager->AddObject(obj);

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

 void CLevelImporter::LoadCamera(tinyxml2::XMLElement* currEntity, CScene* scene)
{
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = scene->mDefaultVs;
	auto pixelShader = scene->mDefaultPs;
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

	scene->mCamera = std::make_unique<CCamera>(pos, rot, FOV, aspectRatio, nearClip, farClip);

	if (!scene->mCamera)
	{
		throw std::runtime_error("Error initializing camera");
	}
}

 void CLevelImporter::LoadPlant(tinyxml2::XMLElement* currEntity, CScene* scene) const
{
	std::string ID;
	std::string mesh;
	std::string name;
	std::string diffuse;
	auto vertexShader = scene->mDefaultVs;
	auto pixelShader = scene->mDefaultPs;

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
			scene->mObjManager->AddObject(obj);
		}
		else
		{
			auto obj = new CPlant(ID, name, vertexShader, pixelShader, pos, rot, scale);
			scene->mObjManager->AddObject(obj);
		}

	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(std::string(e.what()) + " of object " + name);
	}
}

 bool CLevelImporter::ParseEntities(tinyxml2::XMLElement* entitiesEl, CScene* scene)
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

 CLevelImporter::~CLevelImporter()
{

}
