#include "GameObjectManager.h"

#include "SpotLight.h"
#include "PointLight.h"
#include "DirLight.h"
#include "Light.h"
#include "GraphicsHelpers.h"
#include "MathHelpers.h"

extern void DisplayShadowMaps();

CGameObjectManager::CGameObjectManager()
{
	mMaxSize = 100;
	mMaxShadowMaps = 10;
}

void CGameObjectManager::AddObject(CGameObject* obj)
{
	if (mObjects.size() < mMaxSize)
	{
		mObjects.push_back(obj);
	}
	else
	{
		throw std::runtime_error("Not enough space to store more objects");
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
		throw std::runtime_error("Not enough space to store more objects");
	}
}

void CGameObjectManager::AddPointLight(CPointLight* obj)
{
	if (mPointLights.size() < mMaxSize)
	{
		mPointLights.push_back(obj);
	}
	else
		throw std::runtime_error("Not enough space to store more objects");
}

void CGameObjectManager::AddSpotLight(CSpotLight* obj)
{
	if (mSpotLights.size() < mMaxSize)
	{
		mSpotLights.push_back(obj);
	}
	else
		throw std::runtime_error("Not enough space to store more objects");
}

void CGameObjectManager::AddDirLight(CDirLight* obj)
{
	if (mDirLights.size() < mMaxSize)
	{
		mDirLights.push_back(obj);
	}
}

void CGameObjectManager::UpdateLightsBuffer()
{
	auto FCB = &gPerFrameLightsConstants;

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
	auto FLB = &gPerFrameSpotLightsConstants;

	for (auto i = 0; i < mSpotLights.size(); ++i)
	{
		if (*mSpotLights[i]->Enabled())
		{
			FLB->spotLights[i].enabled = 1;
			FLB->spotLights[i].colour = mSpotLights[i]->GetColour();
			FLB->spotLights[i].pos = mSpotLights[i]->Position();
			FLB->spotLights[i].facing = mSpotLights[i]->GetFacing();
			FLB->spotLights[i].cosHalfAngle = cos(ToRadians(mSpotLights[i]->GetConeAngle() / 2));
			FLB->spotLights[i].viewMatrix = InverseAffine(mSpotLights[i]->WorldMatrix());
			FLB->spotLights[i].projMatrix = MakeProjectionMatrix(1.0f, mSpotLights[i]->GetConeAngle());
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

	auto FLB = &gPerFrameDirLightsConstants;

	for (auto i = 0; i < mDirLights.size(); ++i)
	{
		if (*mDirLights[i]->Enabled())
		{
			FLB->dirLights[i].enabled = 1;
			FLB->dirLights[i].colour = mDirLights[i]->GetColour();
			FLB->dirLights[i].facing = mDirLights[i]->GetDirection();
			auto m = mDirLights[i]->WorldMatrix();
			m.SetRow(3, { 0.0f,0.0f,0.0f });
			FLB->dirLights[i].viewMatrix = InverseAffine(m);
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
	for (auto i = 0; i < mPointLights.size(); ++i)
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
			//since they are all the same
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
	for (auto it : mObjects)
	{
		it->RenderToAmbientMap();
	}
}

bool CGameObjectManager::RenderAllObjects()
{
	for (auto it : mObjects)
	{
		it->Render();
	}

	for (auto it : mLights)
	{
		it->Render();
	}

	for (auto it : mSpotLights)
	{
		it->Render();
	}

	for (auto it : mDirLights)
	{
		it->Render();
	}

	for (auto it : mPointLights)
	{
		it->Render();
	}

	if (!gViewportFullscreen)
	{
		DisplayShadowMaps();
	}

	mShadowsMaps.clear();

	// Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
	auto nShadowMaps = mSpotLights.size() + mDirLights.size() + (mPointLights.size() * 6);

	for (int i = 0; i < nShadowMaps; ++i)
	{
		ID3D11ShaderResourceView* nullView = nullptr;
		gD3DContext->PSSetShaderResources(7 + i, 1, &nullView);
	}
	return true;
}

void CGameObjectManager::RenderFromSpotLights()
{
	for (auto it : mSpotLights)
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
	for (auto it : mPointLights)
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
	for (auto it : mDirLights)
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

	for (auto it : mObjects)
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
	for (auto it : mObjects)
	{
		delete it;
	}

	for (auto it : mLights)
	{
		delete it;
	}

	for (auto it : mSpotLights)
	{
		delete it;
	}

	for (auto it : mPointLights)
	{
		delete it;
	}

	for (auto it : mShadowsMaps)
	{
		if (it)
		{
			it->Release();
		}
	}

	mShadowsMaps.clear();
}