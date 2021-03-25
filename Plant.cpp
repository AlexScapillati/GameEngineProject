#include "Plant.h"

#include "Common.h"
#include "State.h"

CPlant::CPlant(CPlant& p) : CGameObject(p)
{
}

void CPlant::Render(bool basicGeometry)
{
	if (basicGeometry)
	{
		CGameObject::Render(basicGeometry);
	}
	else
	{
		// additive blending, read-only depth buffer and no culling (standard set-up for blending)
		gD3DContext->OMSetBlendState(gNoBlendingState, NULL, 0xffffff);
		gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
		gD3DContext->RSSetState(gCullNoneState);

		CGameObject::Render(basicGeometry);
	}
}