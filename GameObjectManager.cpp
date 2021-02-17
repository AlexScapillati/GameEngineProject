#include "GameObjectManager.h"

#include "SpotLight.h"
#include "CPointLight.h"
#include "DirLight.h"
#include "GraphicsHelpers.h"
#include "MathHelpers.h"
#include "External\imgui\imgui.h"

CGameObjectManager::CGameObjectManager()
{
	mMaxSize = 1000;
	mCurrNumLights = 0;
	mCurrNumSpotLights = 0;
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
		mCurrNumLights++;
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
		mCurrNumSpotLights++;
	}
	else
		throw std::runtime_error("Not enough space to store more objects");
}

void CGameObjectManager::AddDirLight(CDirLight* obj)
{
	if (mDirLights.size() < mMaxSize)
	{
		mDirLights.push_back(obj);
		mCurrNumDirLights++;
	}
}


void CGameObjectManager::UpdateLightsConstBuffer(PerFrameLights* FCB)
{
	for (auto i = 0; i < mCurrNumLights; ++i)
	{
		FCB->lights[i].colour = mLights[i]->GetColour() * mLights[i]->GetStrength();
		FCB->lights[i].padding = 1;
		FCB->lights[i].position = mLights[i]->Position();
		FCB->lights[i].numLights = mCurrNumLights;
	}
}

void CGameObjectManager::UpdateSpotLightsConstBuffer(PerFrameSpotLights* FLB)
{
	for (auto i = 0; i < mCurrNumSpotLights; ++i)
	{
		FLB->spotLights[i].colour = mSpotLights[i]->GetColour() * mSpotLights[i]->GetStrength();
		FLB->spotLights[i].pos = mSpotLights[i]->Position();
		FLB->spotLights[i].facing = mSpotLights[i]->GetFacing();
		FLB->spotLights[i].cosHalfAngle = cos(ToRadians(mSpotLights[i]->GetConeAngle() / 2));
		FLB->spotLights[i].viewMatrix = InverseAffine(mSpotLights[i]->WorldMatrix());
		FLB->spotLights[i].projMatrix = MakeProjectionMatrix(1.0f, mSpotLights[i]->GetConeAngle());
		FLB->spotLights[i].numLights = mCurrNumSpotLights;
	}
}

void CGameObjectManager::UpdateDirLightsConstBuffer(PerFrameDirLights* FLB)
{
	for (auto i = 0; i < mCurrNumDirLights; ++i)
	{
		FLB->dirLights[i].colour = mDirLights[i]->GetColour() * mDirLights[i]->GetStrength();
		FLB->dirLights[i].facing = mDirLights[i]->GetMesh()->GetNodeDefaultMatrix(0).GetRow(2);
		FLB->dirLights[i].viewMatrix = InverseAffine(mDirLights[i]->WorldMatrix());
		FLB->dirLights[i].projMatrix = MakeOrthogonalMatrix(1000.0f, 1000.0f, mDirLights[i]->GetNearClip(), mDirLights[i]->GetFarClip());
		FLB->dirLights[i].numLights = mCurrNumDirLights;
	}
}

void CGameObjectManager::UpdatePointLightsConstBuffer(PerFramePointLights* FLB)
{
	for (auto i = 0; i < mPointLights.size(); ++i)
	{
		FLB->pointLights[i].colour = mPointLights[i]->GetColour() * mPointLights[i]->GetStrength();
		FLB->pointLights[i].numLights = mPointLights.size();
		FLB->pointLights[i].position = mPointLights[i]->Position();

		for (auto j = 0; j < 6; ++j)
		{
			mPointLights[i]->WorldMatrix().FaceTarget(mPointLights[i]->mSides[j]);

			FLB->pointLights[i].viewMatrices[j] = InverseAffine(mPointLights[i]->WorldMatrix());

			//probably useless since they are all the same
			FLB->pointLights[i].projMatrices[j] = MakeProjectionMatrix(1.0f, ToRadians(90.0f));
		}
	}
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
		mCurrNumLights--;
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
		mCurrNumSpotLights--;
		return true;
	}

	return false;
}

bool CGameObjectManager::RemoveDirLight(int pos)
{
	if (!mDirLights.empty())
	{
		mDirLights.erase(mDirLights.begin() + pos);
		mCurrNumDirLights--;
		return true;
	}
	return false;
}

extern void DisplayShadowMaps(CGameObjectManager* GOM);

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

	DisplayShadowMaps(this);

	mShadowsMaps.clear();

	// Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
	ID3D11ShaderResourceView* nullView = nullptr;
	gD3DContext->PSSetShaderResources(5, 1, &nullView);

	return true;
}

void CGameObjectManager::RenderFromSpotLights()
{
	for (auto it : mSpotLights)
	{
		if (*it->Enabled())
		{
			//render from its prospective into a texture
			auto temp = it->RenderFromThis(this);

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
			auto tmp = it->RenderFromThis(this);

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
			auto tmp = it->RenderFromThis(this);

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
			delete it;
		}
	}

	mShadowsMaps.clear();
}
