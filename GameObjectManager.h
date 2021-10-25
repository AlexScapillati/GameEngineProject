#pragma once

#include <deque>
#include <memory>
#include <vector>
#include "Common.h"

class CLight;
class CGameObject;
class CSpotLight;
class CDirLight;
class CPointLight;
class CSky;

class CGameObjectManager
{
public:

	CGameObjectManager();

	void AddObject(CGameObject* obj);

	void AddLight(CLight* obj);

	void AddPointLight(CPointLight* obj);

	void AddSpotLight(CSpotLight* obj);

	void AddDirLight(CDirLight* obj);

	void UpdateLightsBuffer();

	void UpdateSpotLightsBuffer();

	void UpdateDirLightsBuffer();

	void UpdatePointLightsBuffer();

	void UpdateAllBuffers();

	bool RemoveObject(int pos);

	bool RemoveLight(int pos);

	bool RemovePointLight(int pos);

	bool RemoveSpotLight(int pos);

	bool RemoveDirLight(int pos);

	void RenderAmbientMaps();

	bool RenderAllObjects();

	void RenderFromSpotLights();

	void RenderFromPointLights();

	void RenderFromDirLights();

	void RenderFromAllLights();

	void UpdateObjects(float updateTime);

	CGameObject* GetSky() { return (CGameObject*)mSky; }

	~CGameObjectManager();

	std::deque<CGameObject*> mObjects;

	std::deque<CLight*> mLights;

	std::deque<CPointLight*> mPointLights;

	std::deque<CSpotLight*> mSpotLights;

	std::deque<CDirLight*> mDirLights;

	std::vector<ID3D11ShaderResourceView*> mShadowsMaps;

private:

	int mMaxSize;

	int mMaxShadowMaps;

	CSky* mSky;
};
