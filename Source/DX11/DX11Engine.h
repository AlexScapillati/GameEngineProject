#pragma once

#include "..\Engine.h"
#include "CDX11Common.h"
#include <d3d11_1.h>

class CDX11Scene;
class CDX11Gui;

class CDX11Engine : public IEngine
{
	//------------------------------------------------
	// Usage
	//------------------------------------------------

public:
	CDX11Engine(HINSTANCE hInstance, int nCmdShow);

	// Inherited via IEngine
	bool Update() override;
	void Finalize() override;

	void Resize(UINT x, UINT y) override;

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetContext() const;
	CDX11Scene* GetScene() const;
	CDX11Gui* GetGui() const;

	~CDX11Engine() override;

	//------------------------------------------------
	// Data
	//------------------------------------------------

	std::unique_ptr<CDX11Scene> mMainScene = nullptr;
	std::unique_ptr<CDX11Gui>   mGui = nullptr;

	// The main Direct3D (D3D) variables
	ComPtr<ID3D11Device>        mD3DDevice  = nullptr; // D3D device for overall features
	ComPtr<ID3D11DeviceContext> mD3DContext = nullptr; // D3D context for specific rendering tasks
	ComPtr<ID3D11Debug>         mD3DDebug   = nullptr;

	// Swap chain and back buffer
	ComPtr<IDXGISwapChain>         mSwapChain              = nullptr;
	ComPtr<ID3D11RenderTargetView> mBackBufferRenderTarget = nullptr;

	// Depth buffer (can also contain "stencil" values, which we will see later)
	ComPtr<ID3D11Texture2D>          mDepthStencilTexture = nullptr; // The texture holding the depth values
	ComPtr<ID3D11DepthStencilView>   mDepthStencil        = nullptr; // The depth buffer referencing above texture
	ComPtr<ID3D11ShaderResourceView> mDepthShaderView     = nullptr;
	// Allows access to the depth buffer as a texture for certain specialised shaders

	//*******************************
	//**** Default shaders shader DirectX objects

	ComPtr<ID3D11PixelShader>  mDepthOnlyPixelShader       = nullptr;
	ComPtr<ID3D11VertexShader> mBasicTransformVertexShader = nullptr;
	ComPtr<ID3D11PixelShader>  mDepthOnlyNormalPixelShader = nullptr;

	ComPtr<ID3D11PixelShader>  mPbrPixelShader        = nullptr;
	ComPtr<ID3D11PixelShader>  mPbrNormalPixelShader  = nullptr;
	ComPtr<ID3D11VertexShader> mPbrVertexShader       = nullptr;
	ComPtr<ID3D11VertexShader> mPbrNormalVertexShader = nullptr;

	ComPtr<ID3D11PixelShader>  mTintedTexturePixelShader = nullptr;
	ComPtr<ID3D11PixelShader>  mSkyPixelShader           = nullptr;
	ComPtr<ID3D11VertexShader> mSkyVertexShader          = nullptr;


	//*******************************
	//**** Post-processing shader DirectX objects

	ComPtr<ID3D11PixelShader> mSsaoPostProcess                = nullptr;
	ComPtr<ID3D11PixelShader> mCopyPostProcess                = nullptr;
	ComPtr<ID3D11PixelShader> mTintPostProcess                = nullptr;
	ComPtr<ID3D11PixelShader> mBurnPostProcess                = nullptr;
	ComPtr<ID3D11PixelShader> mBloomPostProcess               = nullptr;
	ComPtr<ID3D11PixelShader> mSpiralPostProcess              = nullptr;
	ComPtr<ID3D11PixelShader> mGodRaysPostProcess             = nullptr;
	ComPtr<ID3D11PixelShader> mDistortPostProcess             = nullptr;
	ComPtr<ID3D11PixelShader> mSsaoLastPostProcess            = nullptr;
	ComPtr<ID3D11PixelShader> mHeatHazePostProcess            = nullptr;
	ComPtr<ID3D11PixelShader> mBloomLastPostProcess           = nullptr;
	ComPtr<ID3D11PixelShader> mGreyNoisePostProcess           = nullptr;
	ComPtr<ID3D11PixelShader> mGaussionBlurPostProcess        = nullptr;
	ComPtr<ID3D11PixelShader> mChromaticAberrationPostProcess = nullptr;

	ComPtr<ID3D11VertexShader> m2DPolygonVertexShader = nullptr;
	ComPtr<ID3D11VertexShader> m2DQuadVertexShader    = nullptr;


	// GPU "States" //

	// A sampler state object represents a way to filter textures, such as bilinear or trilinear. We have one object for each method we want to use
	ComPtr<ID3D11SamplerState> mPointSampler         = nullptr;
	ComPtr<ID3D11SamplerState> mTrilinearSampler     = nullptr;
	ComPtr<ID3D11SamplerState> mPointSamplerBorder   = nullptr;
	ComPtr<ID3D11SamplerState> mAnisotropic4XSampler = nullptr;

	// Blend states allow us to switch between blending modes (none, additive, multiplicative etc.)
	ComPtr<ID3D11BlendState> mNoBlendingState       = nullptr;
	ComPtr<ID3D11BlendState> mAdditiveBlendingState = nullptr;
	ComPtr<ID3D11BlendState> mAlphaBlendingState    = nullptr;

	// Rasterizer states affect how triangles are drawn
	ComPtr<ID3D11RasterizerState> mCullBackState  = nullptr;
	ComPtr<ID3D11RasterizerState> mCullFrontState = nullptr;
	ComPtr<ID3D11RasterizerState> mCullNoneState  = nullptr;

	// Depth-stencil states allow us change how the depth buffer is used
	ComPtr<ID3D11DepthStencilState> mUseDepthBufferState = nullptr;
	ComPtr<ID3D11DepthStencilState> mDepthReadOnlyState  = nullptr;
	ComPtr<ID3D11DepthStencilState> mNoDepthBufferState  = nullptr;


	void InitDirect3D();

public:
	//--------------------------------------------------------------------------------------
	// Constant buffers
	//--------------------------------------------------------------------------------------

	// Template function to update a constant buffer. Pass the DirectX constant buffer object and the C++ data structure
	// you want to update it with. The structure will be copied in full over to the GPU constant buffer, where it will
	// be available to shaders. This is used to update model and camera positions, lighting data etc.

	void UpdateModelConstantBuffer(ID3D11Buffer* buffer, PerModelConstants& bufferData) const;

	void UpdatePostProcessingConstBuffer(ID3D11Buffer* buffer, PostProcessingConstants& bufferData) const;

	void UpdateFrameConstantBuffer(ID3D11Buffer* buffer, PerFrameConstants& bufferData) const;

	void UpdateLightConstantBuffer(ID3D11Buffer* buffer, PerFrameLights& bufferData, int numLights) const;

	void UpdateSpotLightsConstantBuffer(ID3D11Buffer* buffer, PerFrameSpotLights& bufferData, int numLights) const;

	void UpdateDirLightsConstantBuffer(ID3D11Buffer* buffer, PerFrameDirLights& bufferData, int numLights) const;

	void UpdatePointLightsConstantBuffer(ID3D11Buffer* buffer, PerFramePointLights& bufferData, int numLights) const;

	ID3DBlob* CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements);

	ID3D11Buffer* CreateConstantBuffer(int size);

	//--------------------------------------------------------------------------------------
	// Shaders Functions
	//--------------------------------------------------------------------------------------

	ID3D11VertexShader* LoadVertexShader(const std::string& shaderName);

	ID3D11GeometryShader* LoadGeometryShader(const std::string& shaderName);

	ID3D11GeometryShader* LoadStreamOutGeometryShader(const std::string&          shaderName,
													  D3D11_SO_DECLARATION_ENTRY* soDecl,
													  unsigned int                soNumEntries,
													  unsigned int                soStride);

	ID3D11PixelShader* LoadPixelShader(const std::string& shaderName);

	void LoadDefaultShaders();

	//--------------------------------------------------------------------------------------
	// Texture Loading
	//--------------------------------------------------------------------------------------

	// Using Microsoft's open source DirectX Tool Kit (DirectXTK) to simplify file loading
	// This function requires you to pass a ID3D11Resource* (e.g. &gTilesDiffuseMap), which manages the GPU memory for the
	// texture and also a ID3D11ShaderResourceView* (e.g. &gTilesDiffuseMapSRV), which allows us to use the texture in shaders
	// The function will fill in these pointers with usable data. Returns false on failure

	bool LoadTexture(std::string filename, ID3D11Resource** texture, ID3D11ShaderResourceView** textureSRV);

	bool SaveTextureToFile(ID3D11Resource* tex, std::string& fileName);

	//--------------------------------------------------------------------------------------
	// States creation
	//--------------------------------------------------------------------------------------

	bool CreateStates();
};
