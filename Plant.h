#pragma once

#include <utility>

#include "GameObject.h"

class CPlant : public CGameObject
{
public:

	CPlant(std::string mesh, 
		std::string name, 
		std::string& diffuse,
		CVector3 position = { 0,0,0 }, 
		CVector3 rotation = { 0,0,0 }, 
		float scale = 1)
		: CGameObject(mesh, name, diffuse, position, rotation, scale) {}

	CPlant(std::string id, 
		std::string name, 
		CVector3 position = { 0,0,0 }, 
		CVector3 rotation = { 0,0,0 }, 
		float scale = 1)
		: CGameObject(id, name, position, rotation, scale) {}

	CPlant(CPlant& p);

	void Render(bool basicGeometry = false) override;
};
