#include "DirLight.h"

CDirLight::CDirLight(std::string mesh, std::string name, const std::string& diffuse, std::string& vertexShader, 
	std::string& pixelShader, CVector3 colour, float strength, CVector3 position, CVector3 rotation, float scale, CVector3 facing)
: CLight(mesh,name,diffuse,vertexShader,pixelShader,colour,strength,position,rotation,scale)
{
	mFacing = facing;
	mShadowMap = nullptr;
	mShadowMapDepthStencil = nullptr;
	mShadowMapSRV = nullptr;

	mShadowMapSize = 1024;
}


ID3D11ShaderResourceView* CDirLight::RenderFromThis(CGameObjectManager* CGOM)
{
	return nullptr;
}
