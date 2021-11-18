#include "DX12GameObjectManager.h"
#include "Objects/CDX12Sky.h"

CDX12GameObjectManager::CDX12GameObjectManager(CDX12Engine* engine)
{
	mSky               = nullptr;
	mEngine            = engine;
	mMaxSize           = 100;
	mMaxShadowMaps	   = 10;
}

void CDX12GameObjectManager::AddObject(CDX12GameObject* obj)
{
	if (mObjects.size() < mMaxSize)
	{
		// Try to cast it to the sky object // useful for the ambient map
		if (!mSky)
		{
			mSky = dynamic_cast<CDX12Sky*>(obj);
		}

		mObjects.push_back(obj);
	}
	else
	{
		throw std::exception("Not enough space to store more objects");
	}
}

bool CDX12GameObjectManager::RenderAllObjects()
{
	return false;
}

void CDX12GameObjectManager::RenderFromSpotLights()
{
}

void CDX12GameObjectManager::RenderFromPointLights()
{
}

void CDX12GameObjectManager::RenderFromDirLights()
{
}

void CDX12GameObjectManager::RenderFromAllLights()
{
}

void CDX12GameObjectManager::UpdateObjects(float updateTime)
{
}
