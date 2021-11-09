#pragma once

#include <deque>
#include <vector>
#include <d3d11_1.h>


class CLight;
class CDX11GameObject;
class CSpotLight;
class CDirLight;
class CPointLight;
class CSky;
class CDX11Engine;

class CGameObjectManager
{
public:

	CGameObjectManager(CDX11Engine* engine);

	void AddObject(CDX11GameObject* obj);

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

	CDX11GameObject* GetSky() { return (CDX11GameObject*)mSky; }

	~CGameObjectManager();

	std::deque<CDX11GameObject*> mObjects;

	std::deque<CLight*> mLights;

	std::deque<CPointLight*> mPointLights;

	std::deque<CSpotLight*> mSpotLights;

	std::deque<CDirLight*> mDirLights;

	std::vector<ID3D11ShaderResourceView*> mShadowsMaps;

private:

	CDX11Engine* mEngine;

	int mMaxSize;

	int mMaxShadowMaps;

	CSky* mSky;
};
