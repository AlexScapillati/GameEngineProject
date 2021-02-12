#pragma once
#include <utility>
#include "Common.h"
#include "GameObject.h"
#include "State.h"

class CLight : public CGameObject
{
public:

	CLight(std::string mesh, std::string name,
		const std::string& diffuse, std::string& vertexShader, std::string& pixelShader,
		CVector3 colour = { 0.0f,0.0f,0.0f }, float strength = 0.0f, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1)
		: mColour(colour), mStrength(strength), CGameObject(std::move(mesh), std::move(name), diffuse, vertexShader, pixelShader, position, rotation, scale) {}

	void Render(bool basicGeometry = false) override
	{
		if (basicGeometry)
		{
			//render object
			CGameObject::Render();
		}
		else
		{
			gPerModelConstants.objectColour = mColour;

			//store previous states

			ID3D11BlendState* pBlendState = nullptr;
			ID3D11DepthStencilState* pBPSState = nullptr;
			ID3D11RasterizerState* pRSState = nullptr;

			gD3DContext->OMGetBlendState(&pBlendState, nullptr, nullptr);
			gD3DContext->OMGetDepthStencilState(&pBPSState, 0);
			gD3DContext->RSGetState(&pRSState);

			// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
			gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
			gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
			gD3DContext->RSSetState(gCullNoneState);

			//render object
			CGameObject::Render();

			//set previous states
			gD3DContext->OMSetBlendState(pBlendState, nullptr, 0xffffff);
			gD3DContext->OMSetDepthStencilState(pBPSState, 0);
			gD3DContext->RSSetState(pRSState);
		}

	}

	void SetColour(CVector3 colour)
	{
		mColour = colour;
	}

	void SetStrength(float strength)
	{
		mStrength = strength;
	}

	CVector3 GetColour() const { return mColour; }

	float GetStrength() const { return mStrength; }


private:
	CVector3 mColour;
	float mStrength;
};

