#pragma once

#include "GameObjectManager.h"
#include "Light.h"

class CDirLight :
	public CLight
{

public:

	CDirLight(std::string mesh, std::string name,
		const std::string& diffuse, std::string& vertexShader, std::string& pixelShader,
		CVector3 colour = { 0.0f, 0.0f, 0.0f }, float strength = 0.0f,
		CVector3 position = { 0, 0, 0 }, CVector3 rotation = { 0, 0, 0 }, float scale = 1, CVector3 facing = { 0,0,1 });

	void Render(bool basicGeometry = false) override
	{
		//a directional light is not visible thus not renderable
		return;
	}


	ID3D11ShaderResourceView* RenderFromThis(CGameObjectManager* CGOM);


private:

	int mShadowMapSize;

	CVector3 mFacing;

	ID3D11Texture2D* mShadowMap;
	ID3D11DepthStencilView* mShadowMapDepthStencil;
	ID3D11ShaderResourceView* mShadowMapSRV;

};

