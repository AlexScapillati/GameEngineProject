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

		// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
		gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
		gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
		gD3DContext->RSSetState(gCullNoneState);

		//render object
		CGameObject::Render(false);
	}
}