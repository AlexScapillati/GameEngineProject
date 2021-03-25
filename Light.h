#pragma once
#include <utility>
#include "Common.h"
#include "GameObject.h"
#include "State.h"

/*

Normal point light class
Does not cast shadows

*/

class CLight : public CGameObject
{
public:

	CLight(CLight& l);

	CLight(std::string mesh, std::string name,
		std::string& diffuse,
		CVector3 colour = { 0.0f,0.0f,0.0f }, float strength = 0.0f, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1)
		: mColour(colour), mStrength(strength), CGameObject(std::move(mesh), std::move(name), diffuse, position, rotation, scale) 
	{
		try
		{
			mMaterial->SetVertexShader(gBasicTransformVertexShader);
			mMaterial->SetPixelShader(gTintedTexturePixelShader);
		}
		catch (std::exception e)
		{
			throw std::exception(e.what());
		}

	}

	void Render(bool basicGeometry = false);

	void SetColour(CVector3 colour) { mColour = colour; }

	void SetStrength(float strength) { mStrength = strength; }

	CVector3& GetColour() { return mColour; }

	float& GetStrength() { return mStrength; }

private:
	CVector3 mColour;
	float mStrength;
};
