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
		// Get Previous states
		ID3D11RasterizerState* prevRS = nullptr;
		ID3D11BlendState* prevBS = nullptr;
		ID3D11DepthStencilState* prevDSS = nullptr;
		UINT prevStencilRef = 0;
		UINT prevSampleMask = 0xffffff;
		FLOAT* prevBlendFactor = nullptr;

		gD3DContext->RSGetState(&prevRS);
		gD3DContext->OMGetBlendState(&prevBS,prevBlendFactor,&prevSampleMask);
		gD3DContext->OMGetDepthStencilState(&prevDSS, &prevStencilRef);

		gPerModelConstants.objectColour = mColour;

		// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
		gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
		gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
		gD3DContext->RSSetState(gCullNoneState);

		//render object
		CGameObject::Render(false);

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
