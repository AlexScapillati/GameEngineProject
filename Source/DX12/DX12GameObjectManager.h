#pragma once

#include <vector>

#include "Objects/DX12GameObject.h"
#include "d3d12.h"

class CDX12GameObjectManager
{
public:
		CDX12GameObjectManager(CDX12Engine* engine);
		void AddObject(CDX12GameObject* obj);
		bool RenderAllObjects();
		void RenderFromSpotLights();
		void RenderFromPointLights();
		void RenderFromDirLights();
		void RenderFromAllLights();
		void UpdateObjects(float updateTime);


		std::vector<ID3D12Resource*>  mShadowMaps;
		std::vector<CDX12GameObject*> mObjects;

	private:
		CDX12Engine*     mEngine;
		CDX12GameObject* mSky;
		int              mMaxSize;
		int              mMaxShadowMaps;
};

