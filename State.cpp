//--------------------------------------------------------------------------------------
// State creation
// - Sampler state (Bilinear, trilinear etc.)
// - Blender state (Additive blending, alpha blending etc.)
// - Rasterizer state (Wireframe mode, don't cull back faces etc.)
// - Depth stencil state (How to use the depth and stencil buffer)
//--------------------------------------------------------------------------------------

#pragma once

#include <stdexcept>

#include "DX11Engine.h"


//--------------------------------------------------------------------------------------
// State creation / destruction
//--------------------------------------------------------------------------------------

// Create all the states used in this app, returns true on success
bool CDX11Engine::CreateStates()
{
	//--------------------------------------------------------------------------------------
	// Texture Samplers
	//--------------------------------------------------------------------------------------
	// Each block of code creates a filtering mode. Copy a block and adjust values to add another mode. See texturing lab for details
	D3D11_SAMPLER_DESC samplerDesc = {};

	////-------- Point Sampling (pixelated textures) --------////
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // Point filtering
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;  // Clamp addressing mode for texture coordinates outside 0->1
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;  // --"--
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;  // --"--
	samplerDesc.MaxAnisotropy = 1;                       // Number of samples used if using anisotropic filtering, more is better but max value depends on GPU

	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Controls how much mip-mapping can be used. These settings are full mip-mapping, the usual values
	samplerDesc.MinLOD = 0;                 // --"--

	// Then create a DirectX object for your description that can be used by a shader
	if (FAILED(mD3DDevice->CreateSamplerState(&samplerDesc, &mPointSampler)))
	{
		throw std::runtime_error("Error creating point sampler");

	}

	////-------- Point Sampling with 0 outside 0->1 UV range (border mode) used for shadow maps --------////
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // Point filtering
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;  // Border addressing mode for texture coordinates outside 0->1
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;  // --"--
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;  // --"--
	samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 0;
	samplerDesc.MaxAnisotropy = 1;                       // Number of samples used if using anisotropic filtering, more is better but max value depends on GPU

	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Controls how much mip-mapping can be used. These settings are full mip-mapping, the usual values
	samplerDesc.MinLOD = 0;                 // --"--

	// Then create a DirectX object for your description that can be used by a shader
	if (FAILED(mD3DDevice->CreateSamplerState(&samplerDesc, &mPointSamplerBorder)))
	{
		throw std::runtime_error("Error creating point sampler");

	}

	////-------- Trilinear Sampling --------////
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Point filtering
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;   // Wrap addressing mode for texture coordinates outside 0->1
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;   // --"--
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;   // --"--
	samplerDesc.MaxAnisotropy = 1;                       // Number of samples used if using anisotropic filtering, more is better but max value depends on GPU

	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Controls how much mip-mapping can be used. These settings are full mip-mapping, the usual values
	samplerDesc.MinLOD = 0;                 // --"--

	// Then create a DirectX object for your description that can be used by a shader
	if (FAILED(mD3DDevice->CreateSamplerState(&samplerDesc, &mTrilinearSampler)))
	{
		throw std::runtime_error("Error creating point sampler");

	}

	////-------- Anisotropic filtering --------////
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; // Trilinear filtering
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // Wrap addressing mode for texture coordinates outside 0->1
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    // --"--
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;    // --"--
	samplerDesc.MaxAnisotropy = 4;                        // Number of samples used if using anisotropic filtering, more is better but max value depends on GPU

	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Controls how much mip-mapping can be used. These settings are full mip-mapping, the usual values
	samplerDesc.MinLOD = 0;                 // --"--

	// Then create a DirectX object for your description that can be used by a shader
	if (FAILED(mD3DDevice->CreateSamplerState(&samplerDesc, &mAnisotropic4XSampler)))
	{
		throw std::runtime_error("Error creating anisotropic 4x sampler");

	}

	//--------------------------------------------------------------------------------------
	// Rasterizer States
	//--------------------------------------------------------------------------------------
	// Rasterizer states adjust how triangles are filled in and when they are shown
	// Each block of code creates a rasterizer state. Copy a block and adjust values to add another mode
	D3D11_RASTERIZER_DESC rasterizerDesc = {};

	////-------- Back face culling --------////
	// This is the usual mode - don't show inside faces of objects
	rasterizerDesc.FillMode = D3D11_FILL_SOLID; // Can also set this to wireframe - experiment if you wish
	rasterizerDesc.CullMode = D3D11_CULL_BACK;  // Setting that decides whether the "front" and "back" side of each
															 // triangle is drawn or not. Culling back faces is the norm
	rasterizerDesc.DepthClipEnable = TRUE; // Advanced setting - only used in rare cases

	// Create a DirectX object for the description above that can be used by a shader
	if (FAILED(mD3DDevice->CreateRasterizerState(&rasterizerDesc, &mCullBackState)))
	{
		throw std::runtime_error("Error creating cull-back state");

	}

	////-------- Front face culling --------////
	// This is an unusual mode - it shows inside faces only so the model looks inside-out
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT; // Remove front faces
	rasterizerDesc.DepthClipEnable = TRUE; // Advanced setting - only used in rare cases

	// Create a DirectX object for the description above that can be used by a shader
	if (FAILED(mD3DDevice->CreateRasterizerState(&rasterizerDesc, &mCullFrontState)))
	{
		throw std::runtime_error("Error creating cull-front state");

	}

	////-------- No culling --------////
	// Used for transparent or flat objects - show both sides of faces
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;  // Don't cull any faces
	rasterizerDesc.DepthClipEnable = TRUE; // Advanced setting - only used in rare cases

	// Create a DirectX object for the description above that can be used by a shader
	if (FAILED(mD3DDevice->CreateRasterizerState(&rasterizerDesc, &mCullNoneState)))
	{
		throw std::runtime_error("Error creating cull-none state");

	}

	//--------------------------------------------------------------------------------------
	// Blending States
	//--------------------------------------------------------------------------------------
	// Each block of code creates a filtering mode. Copy a block and adjust values to add another mode. See blending lab for details
	D3D11_BLEND_DESC blendDesc = {};

	////-------- Blending Off State --------////
	blendDesc.RenderTarget[0].BlendEnable = FALSE;              // Disable blending
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;      // How to blend the source (texture colour)
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;     // How to blend the destination (colour already on screen)
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;   // How to combine the above two, almost always ADD

	//** Leave the following settings alone, they are used only in highly unusual cases
	//** Despite the word "Alpha" in the variable names, these are not the settings used for alpha blending
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// Then create a DirectX object for the description that can be used by a shader
	if (FAILED(mD3DDevice->CreateBlendState(&blendDesc, &mNoBlendingState)))
	{
		throw std::runtime_error("Error creating no-blend state");

	}

	////-------- Additive Blending State --------////
	blendDesc.RenderTarget[0].BlendEnable = TRUE;             // Enable blending
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;    // How to blend the source (texture colour)
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;    // How to blend the destination (colour already on screen)
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // How to combine the above two, almost always ADD

	//** Leave the following settings alone, they are used only in highly unusual cases
	//** Despite the word "Alpha" in the variable names, these are not the settings used for alpha blending
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// Then create a DirectX object for the description that can be used by a shader
	if (FAILED(mD3DDevice->CreateBlendState(&blendDesc, &mAdditiveBlendingState)))
	{
		throw std::runtime_error("Error creating additive blending state");

	}

	////-------- Alpha Blending State --------////
	blendDesc.RenderTarget[0].BlendEnable = TRUE;             // Enable blending
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;    // How to blend the source (texture colour)
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;    // How to blend the destination (colour already on screen)
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // How to combine the above two, almost always ADD

	//** Leave the following settings alone, they are used only in highly unusual cases
	//** Despite the word "Alpha" in the variable names, these are not the settings used for alpha blending
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// Then create a DirectX object for the description that can be used by a shader
	if (FAILED(mD3DDevice->CreateBlendState(&blendDesc, &mAlphaBlendingState)))
	{
		throw std::runtime_error("Error creating additive blending state");

	}

	//--------------------------------------------------------------------------------------
	// Depth-Stencil States
	//--------------------------------------------------------------------------------------
	// Depth-stencil states adjust how the depth and stencil buffers are used. The stencil buffer is rarely used so
	// these states are most often used to switch the depth buffer on and off. See depth buffers lab for details
	// Each block of code creates a rasterizer state. Copy a block and adjust values to add another mode
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

	////-------- Enable depth buffer --------////
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;

	// Create a DirectX object for the description above that can be used by a shader
	if (FAILED(mD3DDevice->CreateDepthStencilState(&depthStencilDesc, &mUseDepthBufferState)))
	{
		throw std::runtime_error("Error creating use-depth-buffer state");

	}

	////-------- Enable depth buffer reads only --------////
	// Disables writing to depth buffer - used for transparent objects because they should not be entered in the buffer but do need to check if they are behind something
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Disable writing to depth buffer
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;

	// Create a DirectX object for the description above that can be used by a shader
	if (FAILED(mD3DDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthReadOnlyState)))
	{
		throw std::runtime_error("Error creating depth-read-only state");

	}

	////-------- Disable depth buffer --------////
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;

	// Create a DirectX object for the description above that can be used by a shader
	if (FAILED(mD3DDevice->CreateDepthStencilState(&depthStencilDesc, &mNoDepthBufferState)))
	{
		throw std::runtime_error("Error creating no-depth-buffer state");

	}

	return true;
}

// Release DirectX state objects
void CDX11Engine::ReleaseStates()
{
	if (mPointSampler)           mPointSampler->Release();			mPointSampler = nullptr;
	if (mTrilinearSampler)       mTrilinearSampler->Release();		mTrilinearSampler = nullptr;
	if (mPointSamplerBorder)	 mPointSamplerBorder->Release();	mPointSamplerBorder = nullptr;
	if (mAnisotropic4XSampler)   mAnisotropic4XSampler->Release();	mAnisotropic4XSampler = nullptr;

	if (mUseDepthBufferState)    mUseDepthBufferState->Release();	mUseDepthBufferState = nullptr;
	if (mDepthReadOnlyState)     mDepthReadOnlyState->Release();	mDepthReadOnlyState = nullptr;
	if (mNoDepthBufferState)     mNoDepthBufferState->Release();	mNoDepthBufferState = nullptr;

	if (mCullNoneState)          mCullNoneState->Release();			mCullNoneState = nullptr;
	if (mCullBackState)          mCullBackState->Release();			mCullBackState = nullptr;
	if (mCullFrontState)         mCullFrontState->Release();		mCullFrontState = nullptr;

	if (mNoBlendingState)        mNoBlendingState->Release();		mNoBlendingState = nullptr;
	if (mAlphaBlendingState)     mAlphaBlendingState->Release();	mAlphaBlendingState = nullptr;
	if (mAdditiveBlendingState)  mAdditiveBlendingState->Release();	mAdditiveBlendingState = nullptr;
}