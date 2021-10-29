#include "Plant.h"

#include "Common.h"

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
		mEngine->GetContext()->OMSetBlendState(mEngine->mNoBlendingState, NULL, 0xffffff);
		mEngine->GetContext()->OMSetDepthStencilState(mEngine->mUseDepthBufferState, 0);
		mEngine->GetContext()->RSSetState(mEngine->mCullNoneState);

		CGameObject::Render(basicGeometry);
		
		// Set back the prev states
		mEngine->GetContext()->RSSetState(prevRS);
		mEngine->GetContext()->OMSetBlendState(prevBS, prevBlendFactor, prevSampleMask);
		mEngine->GetContext()->OMSetDepthStencilState(prevDSS, prevStencilRef);

		if (prevRS) prevRS->Release();
		if (prevBS) prevBS->Release();
		if (prevDSS) prevDSS->Release();

		delete[] prevBlendFactor;
	}
}