#pragma once

#include <utility>

#include "GameObject.h"
#include "DDSTextureLoader.h"

class CSky : public CDX11GameObject
{
public:
	CSky(CDX11Engine* engine, std::string mesh,
		std::string name,
		std::string& diffuse,
		CVector3 position = { 0,0,0 },
		CVector3 rotation = { 0,0,0 },
		float scale = 1)
		: CDX11GameObject(engine, std::move(mesh), std::move(name), diffuse, position, rotation, scale)
	{

		// Here the texture gets loaded as a texture cube if is present
		if (diffuse.find("Cube") != std::string::npos && diffuse.find(".dds") != std::string::npos)
		{
			/*DirectX::CreateDDSTextureFromFileEx(
				mEngine->GetDevice(),
				mEngine->GetContext(),
				CA2CT(diffuse.c_str()),
				0,
				D3D11_USAGE_DEFAULT,
				0,
				D3D11_BIND_SHADER_RESOURCE,
				D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE,
				false,
				mMaterial->Texture(),
				mMaterial->TextureSRV());*/

			mIsCubeMap = true;

			mMaterial->SetPixelShader(mEngine->mSkyPixelShader.Get());
			mMaterial->SetVertexShader(mEngine->mSkyVertexShader.Get());

		}
		else
		{
			// Otherwise just set the basic shaders
			mMaterial->SetPixelShader(mEngine->mTintedTexturePixelShader.Get());
			mMaterial->SetVertexShader(mEngine->mBasicTransformVertexShader.Get());

			mIsCubeMap = false;

		}

	}

	CSky(CSky& o) : CDX11GameObject(o)
	{
		auto diffuse = o.mMaterial->TextureFileName();
		// Here the texture gets loaded as a texture cube if is present
		if (diffuse.find("Cube") != std::string::npos && diffuse.find(".dds") != std::string::npos)
		{
			/*DirectX::CreateDDSTextureFromFileEx(
				mEngine->GetDevice(),
				mEngine->GetContext(),
				CA2CT(diffuse.c_str()),
				0,
				D3D11_USAGE_DEFAULT,
				0,
				D3D11_BIND_SHADER_RESOURCE,
				D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE,
				false,
				*mMaterial->Texture(),
				*mMaterial->TextureSRV());*/

			mIsCubeMap = true;

			mMaterial->SetPixelShader(mEngine->mSkyPixelShader.Get());
			mMaterial->SetVertexShader(mEngine->mSkyVertexShader.Get());

		}
		else
		{
			// Otherwise just set the basic shaders
			mMaterial->SetPixelShader(mEngine->mTintedTexturePixelShader.Get());
			mMaterial->SetVertexShader(mEngine->mBasicTransformVertexShader.Get());

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

			mEngine->GetContext()->RSGetState(&pRSState);

			mEngine->GetContext()->OMGetDepthStencilState(&prevDS, &stencilRef);

			mEngine->GetContext()->OMSetDepthStencilState(mEngine->mNoDepthBufferState.Get(), 0);

			//set the colour white for the sky (no tint)
			gPerModelConstants.objectColour = { 1, 1, 1 };

			// skyboxes point inwards
			mEngine->GetContext()->RSSetState(mEngine->mCullFrontState.Get());

			SetPosition(gPerFrameConstants.cameraPosition);

			CDX11GameObject::Render(basicGeometry);

			mEngine->GetContext()->RSSetState(pRSState);

			mEngine->GetContext()->OMSetDepthStencilState(prevDS, stencilRef);

			if (pRSState) pRSState->Release();
			if (prevDS) prevDS->Release();
		}
	}

	bool HasCubeMap() { return mIsCubeMap; }

private:
	
	bool mIsCubeMap;

};

