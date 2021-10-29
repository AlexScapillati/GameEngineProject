#pragma once
#include "GameObjectManager.h"
#include "Light.h"

class CSpotLight :
	public CLight
{
public:
	CSpotLight(CDX11Engine* engine, std::string mesh,
		std::string name,
		std::string& diffuse,
		CVector3 colour = { 0.0f, 0.0f, 0.0f },
		float strength = 0.0f,
		CVector3 position = { 0, 0, 0 },
		CVector3 rotation = { 0, 0, 0 },
		float scale = 1);

	CSpotLight(CSpotLight& s);

	void Render(bool basicGeometry = false) override;

	ID3D11ShaderResourceView* RenderFromThis();

	void SetShadowMapsSize(int value);

	int& GetShadowMapSize() { return mShadowMapSize; }

	void SetConeAngle(float value);

	float& GetConeAngle()
	{
		return mConeAngle;
	}



	void SetRotation(CVector3 rotation, int node = 0) override
	{
		CGameObject::SetRotation(rotation);
	}

	~CSpotLight();

	void Release();

private:

	int mShadowMapSize;
	float mConeAngle;

	void InitTextures();

	ID3D11Texture2D* mShadowMap;
	ID3D11DepthStencilView* mShadowMapDepthStencil;
	ID3D11ShaderResourceView* mShadowMapSRV;
};
