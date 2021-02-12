#include "Plant.h"

#include "Common.h"
#include "State.h"

void CPlant::Render(bool basicGeometry)
{
	if (basicGeometry)
	{
		CGameObject::Render(basicGeometry);
	}
	else
	{
		ID3D11BlendState* pBlendState = nullptr;
		ID3D11DepthStencilState* pBPSState = nullptr;
		ID3D11RasterizerState* pRSState = nullptr;

		gD3DContext->OMGetBlendState(&pBlendState, nullptr, nullptr);
		gD3DContext->OMGetDepthStencilState(&pBPSState, 0);
		gD3DContext->RSGetState(&pRSState);

		// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
		gD3DContext->OMSetBlendState(gNoBlendingState, NULL, 0xffffff);
		gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
		gD3DContext->RSSetState(gCullNoneState);

		CGameObject::Render(basicGeometry);

		//set preavious states
		gD3DContext->OMSetBlendState(pBlendState, nullptr, 0xffffff);
		gD3DContext->OMSetDepthStencilState(pBPSState, 0);
		gD3DContext->RSSetState(pRSState);
		
	}
}
