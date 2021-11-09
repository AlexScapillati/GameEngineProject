#pragma once

#include "GameObject.h"

class CPlant : public CDX11GameObject
{
public:

	CPlant(CDX11Engine* engine, std::string mesh, 
		std::string name, 
		std::string& diffuse,
		CVector3 position = { 0,0,0 }, 
		CVector3 rotation = { 0,0,0 }, 
		float scale = 1)
		: CDX11GameObject(engine, mesh, name, diffuse, position, rotation, scale) {}

	CPlant(CDX11Engine* engine, std::string id,
		std::string name, 
		CVector3 position = { 0,0,0 }, 
		CVector3 rotation = { 0,0,0 }, 
		float scale = 1)
		: CDX11GameObject(engine, id, name, position, rotation, scale) {}

	CPlant(CPlant& p);

	void Render(bool basicGeometry = false) override;
};
