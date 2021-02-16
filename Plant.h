#pragma once

#include <utility>

#include "GameObject.h"

class CPlant : public CGameObject
{
public:

	CPlant(std::string mesh, std::string name,
	       std::string& diffuse, std::string& vertexShader, std::string& pixelShader,
		CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1)
		: CGameObject(std::move(mesh), std::move(name),diffuse,vertexShader,pixelShader, position, rotation, scale) {}

	CPlant(std::string id, std::string name, std::string& vertexShader, std::string& pixelShader, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1)
		: CGameObject(id,name ,vertexShader,pixelShader, position, rotation, scale) {}


	void Render(bool basicGeometry = false) override;

};

