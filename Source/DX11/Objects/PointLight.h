#pragma once
#include "Light.h"

class CPointLight :
	public CLight
{
public:

	CPointLight(CDX11Engine* engine, std::string mesh, std::string name, std::string& diffuse,  CVector3 colour, float strength, CVector3 position = { 0.0f,0.0f,0.0f }, CVector3 rotation = { 0.0f,0.0f,0.0f },
		float scale = 1.0f);

	CPointLight(CPointLight& p);

	void SetShadowMapSize(int size);

	int GetShadowMapSize() { return mShadowMapSize; }

	void Render(bool basicGeometry = false);

	ID3D11ShaderResourceView** RenderFromThis();

	~CPointLight();

	void Release();

	float mSides[6][3] = {          // Starting from facing down the +ve Z direction, left handed rotations
			{ 0.0f,	 0.5f,	0.0f},  // +ve X direction (values multiplied by PI)
			{ 0.0f, -0.5f,	0.0f},  // -ve X direction
			{-0.5f,	 0.0f,	0.0f},  // +ve Y direction
			{ 0.5f,	 0.0f,	0.0f},  // -ve Y direction
			{ 0.0f,	 0.0f,	0.0f},  // +ve Z direction
			{ 0.0f,	 1.0f,  0.0f}   // -ve Z direction
	};

private:

	int mShadowMapSize;

	ID3D11Texture2D* mShadowMap[6];
	ID3D11DepthStencilView* mShadowMapDepthStencils[6];
	ID3D11ShaderResourceView* mShadowMapSRV[6];


	void InitTextures();
};
