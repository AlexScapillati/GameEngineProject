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
		// Get Previous states
		ID3D11RasterizerState* prevRS = nullptr;
		ID3D11BlendState* prevBS = nullptr;
		ID3D11DepthStencilState* prevDSS = nullptr;
		UINT prevStencilRef = 0;
		UINT prevSampleMask = 0xffffff;
		FLOAT* prevBlendFactor = nullptr;

		// additive blending, read-only depth buffer and no culling (standard set-up for blending)
		gD3DContext->OMSetBlendState(gNoBlendingState, NULL, 0xffffff);
		gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
		gD3DContext->RSSetState(gCullNoneState);

		CGameObject::Render(basicGeometry);
		
		// Set back the prev states
		gD3DContext->RSSetState(prevRS);
		gD3DContext->OMSetBlendState(prevBS, prevBlendFactor, prevSampleMask);
		gD3DContext->OMSetDepthStencilState(prevDSS, prevStencilRef);

		if (prevRS) prevRS->Release();
		if (prevBS) prevBS->Release();
		if (prevDSS) prevDSS->Release();

		delete[] prevBlendFactor;
	}
}