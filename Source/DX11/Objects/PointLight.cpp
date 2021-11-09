#include "PointLight.h"
#include "..\Scene.h"

CPointLight::CPointLight(CDX11Engine* engine, std::string mesh, std::string name, std::string& diffuse, CVector3 colour, float strength, CVector3 position, CVector3 rotation, float scale) :
	CLight(engine, std::move(mesh), std::move(name), diffuse, colour, strength, position,
		rotation, scale)
{
	mShadowMapSize = 2048;

	InitTextures();
}

CPointLight::CPointLight(CPointLight& p) : CLight(p)
{
	mShadowMapSize = 2048;

	InitTextures();
}

void CPointLight::SetShadowMapSize(int size)
{
	mShadowMapSize = size;

	InitTextures();
}

void CPointLight::Render(bool basicGeometry)
{
	CLight::Render(basicGeometry);
}

ID3D11ShaderResourceView** CPointLight::RenderFromThis()
{
	// Store original rotation
	const auto originalOrientation = Rotation();

	// Cull none state, if the light is inside an object, the object needs to obstruct the light in every direction
	mEngine->GetContext()->RSSetState(mEngine->mCullNoneState.Get());

	// For every face
	for (int i = 0; i < 6; ++i)
	{
		// Rotate the object
		CVector3 rot = mSides[i];

		SetRotation(rot * PI);

		// Setup the viewport to the size of the shadow map texture
		D3D11_VIEWPORT vp;
		vp.Width = static_cast<FLOAT>(mShadowMapSize);
		vp.Height = static_cast<FLOAT>(mShadowMapSize);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		mEngine->GetContext()->RSSetViewports(1, &vp);
		

		// Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
		// Also clear the the shadow map depth buffer to the far distance
		mEngine->GetContext()->OMSetRenderTargets(0, nullptr, mShadowMapDepthStencils[i]);
		mEngine->GetContext()->ClearDepthStencilView(mShadowMapDepthStencils[i], D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Update the frame buffer with the correct "camera" matrix
		gPerFrameConstants.viewMatrix = InverseAffine(WorldMatrix());
		gPerFrameConstants.projectionMatrix = MakeProjectionMatrix(1.0f, ToRadians(90.0f));
		gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;

		// Update the constant buffer
		mEngine->UpdateFrameConstantBuffer(gPerFrameConstantBuffer.Get(), gPerFrameConstants);

		// Set it to the GPU
		mEngine->GetContext()->VSSetConstantBuffers(1, 1, &gPerFrameConstantBuffer);
		mEngine->GetContext()->PSSetConstantBuffers(1, 1, &gPerFrameConstantBuffer);

		//render just the objects that can cast shadows
		for (const auto it : mEngine->GetScene()->GetObjectManager()->mObjects)
		{
			//basic geometry rendered, that means just render the model's geometry, leaving all the fancy shaders
			it->Render(true);
		}

		// Unbind the shadow map from the render target
		ID3D11DepthStencilView* nullD = nullptr;
		mEngine->GetContext()->OMSetRenderTargets(0, nullptr, nullD);
	}

	// Restore cull back state
	mEngine->GetContext()->RSSetState(mEngine->mCullBackState.Get());

	// Restore original orentation //kind of useless i think
	SetRotation(originalOrientation);

	return mShadowMapSRV;
}

CPointLight::~CPointLight()
{
	CPointLight::Release();
}

void CPointLight::Release()
{
	for (int i = 0; i < 6; ++i)
	{
		if (mShadowMapSRV[i])			mShadowMapSRV[i]->Release();			mShadowMapSRV[i] = nullptr;
		if (mShadowMapDepthStencils[i]) mShadowMapDepthStencils[i]->Release();	mShadowMapDepthStencils[i] = nullptr;
	}
}

void CPointLight::InitTextures()
{
	Release();

	//**** Create Shadow Map texture ****//

	// We also need a depth buffer to go with our portal
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = mShadowMapSize; // Size of the shadow map determines quality / resolution of shadows
	textureDesc.Height = mShadowMapSize;
	textureDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS; // The shadow map contains a single 32-bit value [tech gotcha: have to say typeless because depth buffer and shaders see things slightly differently]
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // Indicate we will use texture as a depth buffer and also pass it to shaders
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (auto& i : mShadowMap)
	{
		if (FAILED(mEngine->GetDevice()->CreateTexture2D(&textureDesc, NULL, &i)))
		{
			throw std::runtime_error("Error creating shadow map texture");
		}
	}

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // See "tech gotcha" above. The depth buffer sees each pixel as a "depth" float
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Flags = 0;

	for (int i = 0; i < 6; ++i)
	{
		if (FAILED(mEngine->GetDevice()->CreateDepthStencilView(mShadowMap[i], &dsvDesc, &mShadowMapDepthStencils[i])))
		{
			throw std::runtime_error("Error creating shadow map depth stencil view");
		}
	}

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // See "tech gotcha" above. The shaders see textures as colours, so shadow map pixels are not seen as depths
											// but rather as "red" floats (one float taken from RGB). Although the shader code will use the value as a depth
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < 6; ++i)
	{
		if (FAILED(mEngine->GetDevice()->CreateShaderResourceView(mShadowMap[i], &srvDesc, &mShadowMapSRV[i])))
		{
			throw std::runtime_error("Error creating shadow map shader resource view");
		}
	}

	for (auto& i : mShadowMap)
	{
		i->Release();
	}
}
