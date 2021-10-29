#include "GameObjectManager.h"

#include "SpotLight.h"
#include "PointLight.h"
#include "DirLight.h"
#include "DX11Gui.h"
#include "Light.h"
#include "Sky.h"
#include "GraphicsHelpers.h"
#include "MathHelpers.h"
#include "GUI.h"

CGameObjectManager::CGameObjectManager(CDX11Engine* engine)
{
	mSky = nullptr;
	mEngine = engine;
	mMaxSize = 100;
	mMaxShadowMaps = 10;
}

void CGameObjectManager::AddObject(CGameObject* obj)
{
	if (mObjects.size() < mMaxSize)
	{
		// Try to cast it to the sky object // useful for the ambient map
		if (!mSky)
		{
			mSky = dynamic_cast<CSky*>(obj);
		}

		mObjects.push_back(obj);
	}
	else
	{
		throw std::exception("Not enough space to store more objects");
	}
}

void CGameObjectManager::AddLight(CLight* obj)
{
	if (mLights.size() < mMaxSize)
	{
		mLights.push_back(obj);
	}
	else
	{
		throw std::exception("Not enough space to store more objects");
	}
}

void CGameObjectManager::AddPointLight(CPointLight* obj)
{
	if (mShadowsMaps.size() < mMaxShadowMaps)
	{
		mPointLights.push_back(obj);
	}
	else
		throw std::exception("Not enough space to store more objects");
}

void CGameObjectManager::AddSpotLight(CSpotLight* obj)
{
	if (mShadowsMaps.size() < mMaxShadowMaps)
	{
		mSpotLights.push_back(obj);
	}
	else
		throw std::exception("Not enough space to store more objects");
}

void CGameObjectManager::AddDirLight(CDirLight* obj)
{
	if (mShadowsMaps.size() < mMaxShadowMaps)
	{
		mDirLights.push_back(obj);
	}
}

void CGameObjectManager::UpdateLightsBuffer()
{
	const auto FCB = &gPerFrameLightsConstants;

	for (auto i = 0; i < mLights.size(); ++i)
	{
		if (*mLights[i]->Enabled())
		{
			FCB->lights[i].colour = mLights[i]->GetColour();
			FCB->lights[i].enabled = 1;
			FCB->lights[i].position = mLights[i]->Position();
			FCB->lights[i].intensity = mLights[i]->GetStrength();
		}
		else
		{
			FCB->lights[i].enabled = 0;
		}
	}
}

void CGameObjectManager::UpdateSpotLightsBuffer()
{
	const auto FLB = &gPerFrameSpotLightsConstants;

	for (auto i = 0; i < mSpotLights.size(); ++i)
	{
		if (*mSpotLights[i]->Enabled())
		{
			FLB->spotLights[i].enabled = 1;
			FLB->spotLights[i].colour = mSpotLights[i]->GetColour();
			FLB->spotLights[i].pos = mSpotLights[i]->Position();
			FLB->spotLights[i].facing =  Normalise(mSpotLights[i]->WorldMatrix().GetRow(2));
			FLB->spotLights[i].cosHalfAngle = cos(ToRadians(mSpotLights[i]->GetConeAngle() / 2));
			FLB->spotLights[i].viewMatrix = InverseAffine(mSpotLights[i]->WorldMatrix());
			FLB->spotLights[i].projMatrix = MakeProjectionMatrix(1.0f, ToRadians(mSpotLights[i]->GetConeAngle()));
			FLB->spotLights[i].intensity = mSpotLights[i]->GetStrength();
		}
		else
		{
			FLB->spotLights[i].enabled = 0;
		}
	}
}

void CGameObjectManager::UpdateDirLightsBuffer()
{
	const auto FLB = &gPerFrameDirLightsConstants;

	for (auto i = 0; i < mDirLights.size(); ++i)
	{
		if (*mDirLights[i]->Enabled())
		{
			FLB->dirLights[i].enabled = 1;
			FLB->dirLights[i].colour = mDirLights[i]->GetColour();
			FLB->dirLights[i].facing = mDirLights[i]->Position();
			FLB->dirLights[i].viewMatrix = InverseAffine(mDirLights[i]->WorldMatrix());
			FLB->dirLights[i].projMatrix = MakeOrthogonalMatrix(mDirLights[i]->GetWidth(), mDirLights[i]->GetWidth(), mDirLights[i]->GetNearClip(), mDirLights[i]->GetFarClip());
			FLB->dirLights[i].intensity = mDirLights[i]->GetStrength();
		}
		else
		{
			FLB->dirLights[i].enabled = 0;
		}
	}
}

void CGameObjectManager::UpdatePointLightsBuffer()
{
	for (unsigned long long i = 0; i < mPointLights.size(); ++i)
	{
		if (*mPointLights[i]->Enabled())
		{
			gPerFramePointLightsConstants.pointLights[i].enabled = 1;
			gPerFramePointLightsConstants.pointLights[i].colour = mPointLights[i]->GetColour();
			gPerFramePointLightsConstants.pointLights[i].intensity = mPointLights[i]->GetStrength();
			gPerFramePointLightsConstants.pointLights[i].position = mPointLights[i]->Position();

			for (auto j = 0; j < 6; ++j)
			{
				CVector3 rot = mPointLights[i]->mSides[j];

				mPointLights[i]->SetRotation(rot * PI);

				gPerFramePointLightsConstants.pointLights[i].viewMatrices[j] = InverseAffine(mPointLights[i]->WorldMatrix());
			}
			//since they are all the same we just need one projection matrix
			gPerFramePointLightsConstants.pointLights[i].projMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.0f));
		}
		else
		{
			gPerFramePointLightsConstants.pointLights[i].enabled = 0;
		}
	}
}

void CGameObjectManager::UpdateAllBuffers()
{
	UpdateLightsBuffer();
	UpdateSpotLightsBuffer();
	UpdateDirLightsBuffer();
	UpdatePointLightsBuffer();

	// Update number of lights
	gPerFrameConstants.nLights = mLights.size();
	gPerFrameConstants.nDirLight = mDirLights.size();
	gPerFrameConstants.nSpotLights = mSpotLights.size();
	gPerFrameConstants.nPointLights = mPointLights.size();
}

bool CGameObjectManager::RemoveObject(int pos)
{
	if (!mObjects.empty())
	{
		mObjects.erase(mObjects.begin() + pos);
		return true;
	}

	return false;
}

bool CGameObjectManager::RemoveLight(int pos)
{
	if (!mLights.empty())
	{
		mLights.erase(mLights.begin() + pos);
		return true;
	}

	return false;
}

bool CGameObjectManager::RemovePointLight(int pos)
{
	if (!mPointLights.empty())
	{
		mPointLights.erase(mPointLights.begin() + pos);
		return true;
	}

	return false;
}

bool CGameObjectManager::RemoveSpotLight(int pos)
{
	if (!mSpotLights.empty())
	{
		mSpotLights.erase(mSpotLights.begin() + pos);
		return true;
	}

	return false;
}

bool CGameObjectManager::RemoveDirLight(int pos)
{
	if (!mDirLights.empty())
	{
		mDirLights.erase(mDirLights.begin() + pos);
		return true;
	}
	return false;
}

void CGameObjectManager::RenderAmbientMaps()
{
	for (const auto it : mObjects)
	{
		it->RenderToAmbientMap();
	}
}

bool CGameObjectManager::RenderAllObjects()
{
	// Firstly render the sky (if any)
	if (mSky) mSky->Render();

	// Render the objects
	for (const auto it : mObjects)
	{
		// If the sky is present, do not render it two times
		if (dynamic_cast<CSky*>(it)) continue;

		// Render the objects
		it->Render();
	}

	// Render the lights 
	for (const auto it : mLights)
	{
		it->Render();
	}

	for (const auto it : mSpotLights)
	{
		it->Render();
	}

	for (const auto it : mDirLights)
	{
		it->Render();
	}

	for (const auto it : mPointLights)
	{
		it->Render();
	}
	
	if (!mEngine->GetGui()->IsSceneFullscreen())
	{
		//dynamic_cast<CDX11Gui*>(mEngine->GetGui())->DisplayShadowMaps();
	}

	mShadowsMaps.clear();

	// Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
	const auto nShadowMaps = mSpotLights.size() + mDirLights.size() + (mPointLights.size() * 6);

	for (int i = 0; i < nShadowMaps; ++i)
	{
		ID3D11ShaderResourceView* nullView = nullptr;
		mEngine->GetContext()->PSSetShaderResources(7 + i, 1, &nullView);
	}
	return true;
}

void CGameObjectManager::RenderFromSpotLights()
{
	for (const auto it : mSpotLights)
	{
		if (*it->Enabled())
		{
			//render from its prospective into a texture
			auto temp = it->RenderFromThis();

			//put this texture in the texture array that will be passed to the shader
			mShadowsMaps.push_back(temp);
		}
	}
}

void CGameObjectManager::RenderFromPointLights()
{
	for (const auto it : mPointLights)
	{
		if (*it->Enabled())
		{
			auto tmp = it->RenderFromThis();

			for (int i = 0; i < 6; ++i)
			{
				mShadowsMaps.push_back(tmp[i]);
			}
		}
	}
}

void CGameObjectManager::RenderFromDirLights()
{
	for (const auto it : mDirLights)
	{
		if (*it->Enabled())
		{
			auto tmp = it->RenderFromThis();

			mShadowsMaps.push_back(tmp);
		}
	}
}

void CGameObjectManager::RenderFromAllLights()
{
	RenderFromSpotLights();
	RenderFromDirLights();
	RenderFromPointLights();
}

void CGameObjectManager::UpdateObjects(float updateTime)
{
	auto pos = 0;

	for (const auto it : mObjects)
	{
		if (!it->Update(updateTime))
		{
			mObjects.erase(mObjects.begin() + pos);
		}
		pos++;
	}
}

CGameObjectManager::~CGameObjectManager()
{
	for (const auto& it : mObjects)
	{
		if (it)delete it;
	}

	for (const auto& it : mLights)
	{
		if (it)delete it;
	}

	for (const auto& it : mSpotLights)
	{
		if (it)delete it;
	}

	for (const auto& it : mPointLights)
	{
		if (it)delete it;
	}

	for (const auto& it : mDirLights)
	{
		if (it) delete it;
	}

	for (const auto& it : mShadowsMaps)
	{
		if (it) it->Release();
	}

	mShadowsMaps.clear();
}