#pragma once

#include <utility>

#include "GameObject.h"
#include "State.h"
#include "Common.h"
#include "DDSTextureLoader.h"

class CSky : public CGameObject
{
public:
	CSky(std::string mesh,
		std::string name,
		std::string& diffuse,
		CVector3 position = { 0,0,0 },
		CVector3 rotation = { 0,0,0 },
		float scale = 1)
		: CGameObject(mesh, name, diffuse, position, rotation, scale)
	{

		// Here the texture gets loaded as a texture cube if is present
		if (diffuse.find("Cube") != std::string::npos && diffuse.find(".dds") != std::string::npos)
		{
			mMaterial->Release();

			DirectX::CreateDDSTextureFromFileEx(
				gD3DDevice.Get(),
				gD3DContext.Get(),
				CA2CT(diffuse.c_str()),
				0,
				D3D11_USAGE_DEFAULT,
				0,
				D3D11_BIND_SHADER_RESOURCE,
				D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE,
				false,
				&mMaterial->Texture(),
				&mMaterial->TextureSRV());

			mIsCubeMap = true;

			mMaterial->SetPixelShader(gSkyPixelShader.Get());
			mMaterial->SetVertexShader(gSkyVertexShader.Get());

		}
		else
		{
			// Otherwise just set the basic shaders
			mMaterial->SetPixelShader(gTintedTexturePixelShader.Get());
			mMaterial->SetVertexShader(gBasicTransformVertexShader.Get());

			mIsCubeMap = false;

		}

	}

	void Render(bool basicGeometry = false) override
	{
		if (basicGeometry)
		{
			// Do not render the sky map in any depth textures
		}
		else
		{

			ID3D11DepthStencilState* prevDS = nullptr;
			UINT stencilRef;
			ID3D11RasterizerState* pRSState = nullptr;

			gD3DContext->RSGetState(&pRSState);

			gD3DContext->OMGetDepthStencilState(&prevDS, &stencilRef);

			gD3DContext->OMSetDepthStencilState(gNoDepthBufferState, 0);

			//set the colour white for the sky (no tint)
			gPerModelConstants.objectColour = { 1, 1, 1 };

			// skyboxes point inwards
			gD3DContext->RSSetState(gCullFrontState);

			SetPosition(gPerFrameConstants.cameraPosition);

			CGameObject::Render(basicGeometry);

			gD3DContext->RSSetState(pRSState);

			gD3DContext->OMSetDepthStencilState(prevDS, stencilRef);

			if (pRSState) pRSState->Release();
			if (prevDS) prevDS->Release();
		}
	}

	bool HasCubeMap() { return mIsCubeMap; }

private:

	bool mIsCubeMap;

};

