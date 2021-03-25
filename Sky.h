#pragma once

#include <utility>

#include "GameObject.h"
#include "State.h"
#include "Common.h"

class CSky : public CGameObject
{
public:
	CSky(std::string mesh, 
		std::string name,
		std::string& diffuse, 
		CVector3 position = { 0,0,0 }, 
		CVector3 rotation = { 0,0,0 }, 
		float scale = 1)
		: CGameObject(mesh, name, diffuse, position, rotation, scale) 
	{
		mMaterial->SetPixelShader(gSkyPixelShader);
	}

	void Render(bool basicGeometry = false) override
	{
		if (basicGeometry)
		{
			CGameObject::Render(basicGeometry);
		}
		else
		{
			//set the colour white for the sky (no tint)
			gPerModelConstants.objectColour = { 1, 1, 1 };

			ID3D11RasterizerState* pRSState = nullptr;

			gD3DContext->RSGetState(&pRSState);

			// Stars point inwards
			gD3DContext->RSSetState(gCullFrontState);

			CGameObject::Render(basicGeometry);

			gD3DContext->RSSetState(pRSState);

			pRSState->Release();
		}
	}
};

