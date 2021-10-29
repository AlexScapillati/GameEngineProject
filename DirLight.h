#pragma once

#include "GameObjectManager.h"
#include "Light.h"

class CDirLight :
	public CLight
{
public:

	CDirLight(CDX11Engine* engine, 
		std::string mesh, 
		std::string name,
		std::string& diffuse, 
		CVector3 colour = { 0.0f, 0.0f, 0.0f }, 
		float strength = 0.0f,
		CVector3 position = { 0, 0, 0 }, 
		CVector3 rotation = { 0, 0, 0 }, 
		float scale = 1);

	CDirLight(CDirLight& l);

	ID3D11ShaderResourceView* RenderFromThis();

	auto GetNearClip() { return mNearClip; }
	auto GetFarClip() { return mFarClip; }
	auto SetNearClip(float n) { mNearClip = n; }
	auto SetFarClip(float n) { mFarClip = n; }
	auto GetWidth() { return mWidth; }
	auto GetHeight() { return mHeight; }
	auto SetWidth(float n) { mWidth = n; }
	auto SetHeight(float n) { mHeight = n; }
	void SetShadowMapSize(int s);
	auto GetShadowMapSize() { return mShadowMapSize; }

	~CDirLight();

	void Release();

	void InitTextures();

private:

	int mShadowMapSize;

	float mWidth;
	float mHeight;
	float mNearClip;
	float mFarClip;

	ID3D11Texture2D* mShadowMap;
	ID3D11DepthStencilView* mShadowMapDepthStencil;
	ID3D11ShaderResourceView* mShadowMapSRV;
};
