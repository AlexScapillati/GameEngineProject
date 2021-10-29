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

		mEngine->GetContext()->RSGetState(&prevRS);
		mEngine->GetContext()->OMGetBlendState(&prevBS,prevBlendFactor,&prevSampleMask);
		mEngine->GetContext()->OMGetDepthStencilState(&prevDSS, &prevStencilRef);

		gPerModelConstants.objectColour = mColour;

		// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
		mEngine->GetContext()->OMSetBlendState(mEngine->mAdditiveBlendingState, nullptr, 0xffffff);
		mEngine->GetContext()->OMSetDepthStencilState(mEngine->mDepthReadOnlyState, 0);
		mEngine->GetContext()->RSSetState(mEngine->mCullNoneState);

		//render object
		CGameObject::Render(false);

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
