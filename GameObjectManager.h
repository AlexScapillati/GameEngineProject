
#pragma once

#include "GameObject.h"
#include "Light.h"
#include <deque>
#include <memory>

#include "Common.h"

class CSpotLight;
class CDirLight;

class CGameObjectManager
{
public:

	CGameObjectManager();

	void AddObject(CGameObject* obj);

	void AddLight(CLight* obj);

	void AddSpotLight(CSpotLight* obj);

	void AddDirLight(CDirLight* obj);

	void UpdateLightsConstBuffer(PerFrameLights* FCB);
	
	void UpdateSpotLightsConstBuffer(PerFrameSpotLights* FLB);

	void UpdateDirLightsConstBuffer(PerFrameDirLights* FLB);

	bool RemoveObject(int pos);

	bool RemoveLight(int pos);

	bool RemoveSpotLight(int pos);

	bool RemoveDirLight(int pos);
	
	bool RenderAllObjects();

	void RenderFromSpotLights();

	void RenderFromDirLights();

	void UpdateObjects(float updateTime);

	~CGameObjectManager();

	std::deque<CGameObject*> mObjects;

	std::deque<CLight*> mLights;

	std::deque<CSpotLight*> mSpotLights;

	std::deque<CDirLight*> mDirLights;

	std::deque<ID3D11ShaderResourceView*> mShadowsMaps;
	

private:

	int mMaxSize;
	int mCurrNumSpotLights;
	int mCurrNumLights;
	int mCurrNumDirLights;

};

