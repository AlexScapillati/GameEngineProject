#include "Plant.h"


CPlant::CPlant(CPlant& p) : CDX11GameObject(p)
{
}

void CPlant::Render(bool basicGeometry)
{
	if (basicGeometry)
	{
		CDX11GameObject::Render(basicGeometry);
	}
	else
	{
		// Get Previous states
		ID3D11RasterizerState* prevRS	 = nullptr;
		ID3D11BlendState* prevBS		 = nullptr;
		ID3D11DepthStencilState* prevDSS = nullptr;
		UINT prevStencilRef				 = 0;
		UINT prevSampleMask				 = 0xffffff;
		FLOAT* prevBlendFactor			 = nullptr;

		// additive blending, read-only depth buffer and no culling (standard set-up for blending)
		mEngine->GetContext()->OMSetBlendState(mEngine->mNoBlendingState.Get(), NULL, 0xffffff);
		mEngine->GetContext()->OMSetDepthStencilState(mEngine->mUseDepthBufferState.Get(), 0);
		mEngine->GetContext()->RSSetState(mEngine->mCullNoneState.Get());

		CDX11GameObject::Render(basicGeometry);
		
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