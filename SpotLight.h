#pragma once
#include "GameObjectManager.h"
#include "Light.h"

class CSpotLight :
    public CLight
{
	
public:
	CSpotLight(std::string mesh, std::string name,
	          const std::string& diffuse, std::string& vertexShader, std::string& pixelShader,
	          CVector3 colour = {0.0f, 0.0f, 0.0f}, float strength = 0.0f,
		CVector3 position = { 0, 0, 0 }, CVector3 rotation = { 0, 0, 0 }, float scale = 1, CVector3 facing = { 0,0,1 });

	void Render(bool basicGeometry = false) override;


	ID3D11ShaderResourceView* RenderFromThis(CGameObjectManager* CGOM);

	void SetShadowMapsSize(int value);

	int getShadowMapSize() { return mShadowMapSize; }

	void SetConeAngle(float value);

	float GetConeAngle() const
	{
		return mConeAngle;
	}

	CVector3 GetFacing() const
	{
		return mFacing;
	}

	void SetFacing(CVector3 v)
	{
		mFacing = v;
		mWorldMatrices[0].FaceTarget(Position()+v);
	}
	
	void SetRotation(CVector3 rotation, int node = 0) override
	{
		CGameObject::SetRotation(rotation);
		//mFacing = Normalise(Rotation());
	}

	~CSpotLight();

private:

	int mShadowMapSize;
	float mConeAngle;
	CVector3 mFacing;
	

	ID3D11Texture2D* mShadowMap;
	ID3D11DepthStencilView* mShadowMapDepthStencil;
	ID3D11ShaderResourceView* mShadowMapSRV;
	
};

