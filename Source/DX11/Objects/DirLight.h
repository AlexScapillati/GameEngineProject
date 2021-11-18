#pragma once

#include <d3d11.h>

#include "Light.h"

class CVector3;
class CDX11Engine;

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

	auto GetNearClip() const { return mNearClip; }
	auto GetFarClip() const { return mFarClip; }
	auto SetNearClip(float n) { mNearClip = n; }
	auto SetFarClip(float n) { mFarClip = n; }
	auto GetWidth() const { return mWidth; }
	auto GetHeight() const { return mHeight; }
	auto SetWidth(float n) { mWidth = n; }
	auto SetHeight(float n) { mHeight = n; }
	void SetShadowMapSize(int s);
	auto GetShadowMapSize() const { return mShadowMapSize; }

	~CDirLight() override;

	void Release() const;

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
