#pragma once


#include "GameObject.h"

/*

Normal point light class
Does not cast shadows

*/

class CLight : public CDX11GameObject
{
public:

	CLight(CLight& l);

	CLight(CDX11Engine* engine, 
		std::string mesh, 
		std::string name,
		std::string& diffuse,
		CVector3 colour = { 0.0f,0.0f,0.0f }, 
		float strength = 0.0f, 
		CVector3 position = { 0,0,0 },
		CVector3 rotation = { 0,0,0 }, 
		float scale = 1);

	void Render(bool basicGeometry = false);

	void SetColour(CVector3 colour) { mColour = colour; }

	void SetStrength(float strength) { mStrength = strength; }

	CVector3& GetColour() { return mColour; }

	float& GetStrength() { return mStrength; }

private:
	CVector3 mColour;
	float mStrength;
};
