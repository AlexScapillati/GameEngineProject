#include "DX11Engine.h"
#include "..\Window.h"
#include "DX11Scene.h"

#include <stdexcept>
#include <sstream>

#include "DX11Gui.h"
#include "../Utility/Input.h"
#include "..\Common/Camera.h"

//--------------------------------------------------------------------------------------
// Initialise Direct3D
//--------------------------------------------------------------------------------------

void CDX11Engine::InitDirect3D()
{
	// Many DirectX functions return a "HRESULT" variable to indicate success or failure. Microsoft code often uses
	// the FAILED macro to test this variable, you'll see it throughout the code - it's fairly self explanatory.
	auto hr = S_OK;

	//// Initialise DirectX ////

	// Create a Direct3D device (i.e. initialise D3D) and create a swap-chain (create a back buffer to render to)
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.OutputWindow         = mWindow->GetHandle(); // Target window
	swapDesc.SwapEffect           = DXGI_SWAP_EFFECT_DISCARD;
	// Replace this with the better DXGI_SWAP_EFFECT_FLIP_DISCARD for Windows 10 and up (but won't work on earlier Windows)
	swapDesc.Windowed                           = TRUE;
	swapDesc.BufferCount                        = 1;
	swapDesc.BufferDesc.Width                   = mWindow->GetWindowWidth(); // Target window size
	swapDesc.BufferDesc.Height                  = mWindow->GetWindowHeight(); // --"--
	swapDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format of target window
	swapDesc.BufferDesc.RefreshRate.Numerator   = 60; // Refresh rate of monitor (provided as fraction = 60/1 here)
	swapDesc.BufferDesc.RefreshRate.Denominator = 1; // --"--
	swapDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.SampleDesc.Count                   = 1;
	swapDesc.SampleDesc.Quality                 = 0;

	// Set this to 0, or D3D11_CREATE_DEVICE_DEBUG to get more debugging information (in the "Output" window of Visual Studio)
	UINT flags = D3D11_CREATE_DEVICE_DEBUG;

	hr = D3D11CreateDeviceAndSwapChain(nullptr,
									   D3D_DRIVER_TYPE_HARDWARE,
									   0,
									   flags,
									   0,
									   0,
									   D3D11_SDK_VERSION,
									   &swapDesc,
									   mSwapChain.GetAddressOf(),
									   mD3DDevice.GetAddressOf(),
									   nullptr,
									   mD3DContext.GetAddressOf());
	if (FAILED(hr)) { throw std::runtime_error("Error creating Direct3D device"); }

	// Get a "render target view" of back-buffer - standard behaviour
	ID3D11Texture2D* backBuffer;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr)) { throw std::runtime_error("Error creating swap chain"); }

	hr = mD3DDevice->CreateRenderTargetView(backBuffer, NULL, mBackBufferRenderTarget.GetAddressOf());
	backBuffer->Release();
	if (FAILED(hr)) { throw std::runtime_error("Error creating render target view"); }

	//create debug device
	hr = mD3DDevice->QueryInterface(IID_PPV_ARGS(mD3DDebug.GetAddressOf()));
	if (FAILED(hr)) { throw std::runtime_error("Unable to create debug device"); }

	//// Create depth buffer to go along with the back buffer ////

	// First create a texture to hold the depth buffer values
	D3D11_TEXTURE2D_DESC dbDesc = {};
	dbDesc.Width                = mWindow->GetWindowWidth(); // Same size as viewport / back-buffer
	dbDesc.Height               = mWindow->GetWindowHeight();
	dbDesc.MipLevels            = 1;
	dbDesc.ArraySize            = 1;
	dbDesc.Format               = DXGI_FORMAT_R32_TYPELESS; // Each depth value is a single float
	// Important point for when using depth buffer as texture, must use the TYPELESS constant shown here
	dbDesc.SampleDesc.Count   = 1;
	dbDesc.SampleDesc.Quality = 0;
	dbDesc.Usage              = D3D11_USAGE_DEFAULT;
	dbDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	// Using this depth buffer in shaders so must say so;
	dbDesc.CPUAccessFlags = 0;
	dbDesc.MiscFlags      = 0;
	hr                    = mD3DDevice->CreateTexture2D(&dbDesc, nullptr, mDepthStencilTexture.GetAddressOf());
	if (FAILED(hr)) { throw std::runtime_error("Error creating depth buffer texture"); }

	// Create the depth stencil view - an object to allow us to use the texture
	// just created as a depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
	// Important point for when using depth buffer as texture, ensure you use this setting - different from other labs
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = mD3DDevice->CreateDepthStencilView(mDepthStencilTexture.Get(), &dsvDesc, mDepthStencil.GetAddressOf());
	if (FAILED(hr)) { throw std::runtime_error("Error creating depth buffer view"); }

	// Also create a shader resource view for the depth buffer - required when we want to access the depth buffer as a texture (also note the two important comments in above code)
	// Note the veryt
	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	descSRV.Format = DXGI_FORMAT_R32_FLOAT;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = 1;
	descSRV.Texture2D.MostDetailedMip = 0;
	hr = mD3DDevice->CreateShaderResourceView(mDepthStencilTexture.Get(), &descSRV, mDepthShaderView.GetAddressOf());
	if (FAILED(hr)) { throw std::runtime_error("Error creating depth buffer shader resource view"); }
}

CDX11Engine::CDX11Engine(HINSTANCE hInstance, int nCmdShow)
{
	try
	{
		// Create a window 
		mWindow = std::make_unique<CWindow>(hInstance, nCmdShow);
	}
	catch (const std::exception& e) { throw std::runtime_error(e.what()); }

	//get the executable path
	CHAR path[MAX_PATH];

	GetModuleFileNameA(hInstance, path, MAX_PATH);

	const auto pos = std::string(path).find_last_of("\\/");

	//get the media folder
	mMediaFolder = std::string(path).substr(0, pos) + "/Media/";

	// Prepare TL-Engine style input functions
	InitInput();

	try
	{
		// Initialise Direct3D
		InitDirect3D();
	}
	catch (const std::runtime_error& e) { throw std::runtime_error(e.what()); }

	try
	{
		// Create scene
		mMainScene = std::make_unique<CDX11Scene>(this, "Scene2.xml");
	}
	catch (const std::exception& e) { throw std::runtime_error(e.what()); }

	mGui = std::make_unique<CDX11Gui>(this);

	// Will use a timer class to help in this tutorial (not part of DirectX). It's like a stopwatch - start it counting now
	mTimer.Start();
}


bool CDX11Engine::Update()
{
	// Main message loop - this is a Windows equivalent of the loop in a TL-Engine application
	MSG msg = {};
	while (msg.message != WM_QUIT) // As long as window is open
	{
		// Check for and deal with any window messages (input, window resizing, minimizing, etc.).
		// The actual message processing happens in the function WndProc below
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// Deal with messages
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else // When no windows messages left to process then render & update our scene
		{
			// Update the scene by the amount of time since the last frame
			auto frameTime = mTimer.GetLapTime();

			mGui->Begin(frameTime);

			mMainScene->UpdateScene(frameTime);

			// Draw the scene
			mMainScene->RenderScene(frameTime);

			mGui->Show(frameTime);

			////--------------- Scene completion ---------------////

			// When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
			// Set first parameter to 1 to lock to vsync
			if (mSwapChain->Present(mMainScene->mLockFPS ? 1 : 0, 0) == DXGI_ERROR_DEVICE_REMOVED)
			{
				const auto reason = mD3DDevice->GetDeviceRemovedReason();

				throw std::runtime_error("Device Removed" + std::to_string(reason));
			}

			if (KeyHit(Key_Escape))
			{
				// Ask to save // WIP 
				//ImGui::OpenPopup("Save?");

				// Save automatically
				mMainScene->Save();

				return false;
			}
		}
	}

	return (int)msg.wParam;
}

void CDX11Engine::Finalize()
{
	// Set the back buffer as the target for rendering and select the main depth buffer.
	// When finished the back buffer is sent to the "front buffer" - which is the monitor.
	mD3DContext->OMSetRenderTargets(1, mBackBufferRenderTarget.GetAddressOf(), mDepthStencil.Get());

	// Clear the back buffer to a fixed colour and the depth buffer to the far distance
	mD3DContext->ClearRenderTargetView(mBackBufferRenderTarget.Get(), &mMainScene->mBackgroundColor.r);
	mD3DContext->ClearDepthStencilView(mDepthStencil.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void CDX11Engine::Resize(UINT x, UINT y)
{
	if (mSwapChain)
	{
		// Check if the window is not minimized
		if (x > 1 && y > 1)
		{
			// Calculate overall dimensions for the window.
			RECT rc;

			GetUpdateRect(mWindow->GetHandle(), &rc, false);

			mWindow->SetWindowSize(rc.right, rc.bottom);

			mMainScene->Resize(x, y);

			HRESULT hr;

			mD3DContext->OMSetRenderTargets(0, 0, 0);

			// Release all outstanding references to the swap chain's buffers.
			mBackBufferRenderTarget.Reset();

			mDepthShaderView.Reset();
			mDepthStencil.Reset();
			mDepthStencilTexture.Reset();

			//// Create depth buffer to go along with the back buffer ////

			// First create a texture to hold the depth buffer values
			D3D11_TEXTURE2D_DESC dbDesc = {};
			dbDesc.Width                = x; // Same size as viewport / back-buffer
			dbDesc.Height               = y;
			dbDesc.MipLevels            = 1;
			dbDesc.ArraySize            = 1;
			dbDesc.Format               = DXGI_FORMAT_R32_TYPELESS; // Each depth value is a single float
			// Important point for when using depth buffer as texture, must use the TYPELESS constant shown here
			dbDesc.SampleDesc.Count   = 1;
			dbDesc.SampleDesc.Quality = 0;
			dbDesc.Usage              = D3D11_USAGE_DEFAULT;
			dbDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			// Using this depth buffer in shaders so must say so;
			dbDesc.CPUAccessFlags = 0;
			dbDesc.MiscFlags      = 0;
			hr                    = mD3DDevice->CreateTexture2D(&dbDesc, nullptr, mDepthStencilTexture.GetAddressOf());
			if (FAILED(hr)) { throw std::runtime_error("Error creating depth buffer texture"); }

			// Create the depth stencil view - an object to allow us to use the texture
			// just created as a depth buffer
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
			// Important point for when using depth buffer as texture, ensure you use this setting - different from other labs
			dsvDesc.Texture2D.MipSlice = 0;
			hr = mD3DDevice->CreateDepthStencilView(mDepthStencilTexture.Get(), &dsvDesc, mDepthStencil.GetAddressOf());
			if (FAILED(hr)) { throw std::runtime_error("Error creating depth buffer view"); }

			// Also create a shader resource view for the depth buffer - required when we want to access the depth buffer as a texture (also note the two important comments in above code)
			// Note the veryt
			D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
			descSRV.Format                    = DXGI_FORMAT_R32_FLOAT;
			descSRV.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
			descSRV.Texture2D.MipLevels       = 1;
			descSRV.Texture2D.MostDetailedMip = 0;
			hr                                = mD3DDevice->CreateShaderResourceView(mDepthStencilTexture.Get(),
																					 &descSRV,
																					 mDepthShaderView.GetAddressOf());
			if (FAILED(hr)) { throw std::runtime_error("Error creating depth buffer shader resource view"); }

			// Preserve the existing buffer count and format.
			// Automatically choose the width and height to match the client rect for HWNDs.
			hr = mSwapChain->ResizeBuffers(1, x, y, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_CREATE_DEVICE_DEBUG);
			if (FAILED(hr)) { throw std::runtime_error("Error resizing buffer"); }

			// Get buffer and create a render-target-view.
			ID3D11Texture2D* pBuffer;
			hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
			if (FAILED(hr)) { throw std::runtime_error("Error getting buffer "); }

			hr = mD3DDevice->CreateRenderTargetView(pBuffer, NULL, mBackBufferRenderTarget.GetAddressOf());
			if (FAILED(hr)) { throw std::runtime_error("Error creating render target view"); }

			pBuffer->Release();
		}
	}
}

ID3D11Device* CDX11Engine::GetDevice() const
{ return mD3DDevice.Get(); }

ID3D11DeviceContext* CDX11Engine::GetContext() const
{ return mD3DContext.Get(); }

CDX11Scene* CDX11Engine::GetScene() const
{ return mMainScene.get(); }

CDX11Gui* CDX11Engine::GetGui() const
{ return mGui.get(); }


CDX11Engine::~CDX11Engine()
{
	// Release each Direct3D object to return resources to the system. Leaving these out will cause memory
	// leaks. Check documentation to see which objects need to be released when adding new features in your
	// own projects.
	if (mD3DContext)
	{
		mD3DContext->ClearState(); // This line is also needed to reset the GPU before shutting down DirectX
		mD3DContext->Flush();
		mD3DContext = nullptr;
	}
}
