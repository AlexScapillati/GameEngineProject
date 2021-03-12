#include "Light.h"

CLight::CLight(CLight& l) : CGameObject(l)
{
	mColour = l.GetColour();
	mStrength = l.GetStrength();
}

void CLight::Render(bool basicGeometry)
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
		CGameObject::Render(false);

		//set previous states
		gD3DContext->OMSetBlendState(pBlendState, nullptr, 0xffffff);
		gD3DContext->OMSetDepthStencilState(pBPSState, 0);
		gD3DContext->RSSetState(pRSState);
	}
}