#pragma once
#include <utility>

#include "GameObject.h"
class CParticle :
    public CGameObject
{
public:

	CParticle(std::string mesh,std::string name,
	          const std::string& diffuse, std::string& vertexShader, std::string& pixelShader,
		CVector3 colour = { 0.0f,0.0f,0.0f }, float strength = 0.0f, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1)
		: CGameObject(std::move(mesh), std::move(name),diffuse,vertexShader,pixelShader, position, rotation, scale) {}

private:

	ID3D11InputLayout* ParticleLayout;
	ID3D11Buffer* ParticleVertexBuffer;

};

