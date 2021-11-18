#include "DX12Engine.h"


// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif


#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <atlconv.h>

#include <d3d12.h>

#include "d3dx12.h"
#include <DirectXTex.h>
#include <dxgi1_6.h>

#include "../Utility/Input.h"
#include "..\Window.h"

#include "DX12Scene.h"
#include "DX12Gui.h"



CDX12Engine::CDX12Engine(HINSTANCE hInstance, int nCmdShow)
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
		InitD3D();
	}
	catch (const std::runtime_error& e) { throw std::runtime_error(e.what()); }

	try
	{
		// Create scene
		mMainScene = std::make_unique<CDX12Scene>(this, "Scene2.xml");
	}
	catch (const std::exception& e) { throw std::runtime_error(e.what()); }

	mGui = std::make_unique<CDX12Gui>(this);

	// Will use a timer class to help in this tutorial (not part of DirectX). It's like a stopwatch - start it counting now
	mTimer.Start();
}

bool CDX12Engine::Update()
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

			//mGui Will call the finalize function which fill present the back buffer to the screen
			mGui->Show(frameTime);

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


	Flush();

	return (int)msg.wParam;
}


void CDX12Engine::CheckRayTracingSupport() const
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};

	auto hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));

	if (hr != S_OK) throw std::runtime_error("Error");

	if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) throw std::runtime_error("RayTracing Not Supported");
}

void CDX12Engine::EnableDebugLayer()
{
#if defined(_DEBUG)

	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;

	if ((D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)) != S_OK))
	{
		throw std::runtime_error("Impossible to enable debug layer");
	}

	debugInterface->EnableDebugLayer();

#endif
}


ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
{
	ComPtr<IDXGIFactory4> dxgiFactory;

	UINT createFactoryFlags = 0;

#if defined(_DEBUG)

	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

#endif

	if ((CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory))))
	{
		throw std::runtime_error("Could not get debug adapter");
	}

	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (useWarp)
	{
		if (dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)))
		{
			throw std::runtime_error("Could not get warp adapter");
		}

		if (dxgiAdapter1.As(&dxgiAdapter4)) { throw std::runtime_error("Could not get adapter"); }
	}
	else
	{
		SIZE_T maxDedicatedVideoMemory = 0;

		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			// Check to see if the adapter can create a D3D12 device without actually 
			// creating it. The adapter with the largest dedicated video memory
			// is favored.

			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr
				)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				if (dxgiAdapter1.As(&dxgiAdapter4)) { throw std::runtime_error("Could not get adapter"); }
			}
		}
	}
	return dxgiAdapter4;
}

ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
{
	ComPtr<ID3D12Device2> d3d12Device2;
	if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2))))
	{
		throw std::runtime_error("Error Creating device");
	}

	// Enable debug messages in debug mode.

#if defined(_DEBUG)

	ComPtr<ID3D12InfoQueue> pInfoQueue;

	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);

		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages

		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level

		D3D12_MESSAGE_SEVERITY Severities[] =

		{

			D3D12_MESSAGE_SEVERITY_INFO

		};

		// Suppress individual messages by their ID

		D3D12_MESSAGE_ID DenyIds[] = {

			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			// I'm really not sure how to avoid this message.

			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			// This warning occurs when using capture frame while graphics debugging.

			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			// This warning occurs when using capture frame while graphics debugging.

		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};

		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;

		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		if (pInfoQueue->PushStorageFilter(&NewFilter))
		{
			throw std::runtime_error("Error");
		}
	}

#endif

	return d3d12Device2;
}

void CDX12Engine::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	if (FAILED(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue))))
	{
		throw std::runtime_error("Error creating command queue");
	}

	mCommandQueue = d3d12CommandQueue;
}


bool CheckTearingSupport()
{
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing))))
			{
				allowTearing = FALSE;
			}
		}
	}

	return allowTearing == TRUE;
}

ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd,
	ComPtr<ID3D12CommandQueue> commandQueue,
	uint32_t width, uint32_t height, uint32_t bufferCount)
{
	ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	if (CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)))
	{
		throw std::runtime_error("Error create DXGIFactory");
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	if (dxgiFactory4->CreateSwapChainForHwnd(
		commandQueue.Get(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1))
	{
		throw std::runtime_error("Error creating swap chain");
	}

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	if (dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER))
	{
		throw std::runtime_error("Error");
	}

	if (swapChain1.As(&dxgiSwapChain4))
	{
		throw std::runtime_error("Error casting swap chain");
	}

	return dxgiSwapChain4;
}


void CDX12Engine::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;

	if (mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)))
	{
		throw std::runtime_error("Error creating descriptor heap");
	}

	mRTVDescriptorHeap = descriptorHeap;
}

void CDX12Engine::UpdateRenderTargetViews()
{
	auto rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < mNumFrames; ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
		if (mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)))
		{
			throw std::runtime_error("Error getting the current swap chain buffer");
		}

		mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

		mBackBuffers[i] = backBuffer;

		rtvHandle.Offset(rtvDescriptorSize);
	}
}

ComPtr<ID3D12CommandAllocator> CDX12Engine::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) const
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;

	if (mDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)))
	{
		throw std::runtime_error("Error creating command allocator");
	}

	return commandAllocator;
}

void CDX12Engine::CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12GraphicsCommandList2> commandList;
	if (mDevice->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)))
	{
		throw std::runtime_error("Error creating command list");
	}

	if (commandList->Close())
	{
		throw std::runtime_error("Error closing command list");
	}

	mCommandList = commandList;
}

ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device)
{
	ComPtr<ID3D12Fence> fence;

	if (device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))
	{
		throw std::runtime_error("Error creating fence");
	}

	return fence;
}

HANDLE CreateEventHandle()
{
	HANDLE fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent && "Failed to create fence event.");

	return fenceEvent;
}


uint64_t CDX12Engine::Signal()
{
	uint64_t fenceValueForSignal = ++mFenceValue;
	if (mCommandQueue->Signal(mFence.Get(), fenceValueForSignal))
	{
		throw std::runtime_error("Error");
	}

	return fenceValueForSignal;
}


Resource CDX12Engine::LoadTexture(std::string& filename) const
{

	filename = mMediaFolder + filename;

	DirectX::TexMetadata   metadata{};
	DirectX::ScratchImage  scratchImage;
	ComPtr<ID3D12Resource> textureResource;


	// DDS files need a different function from other files
	std::string dds = ".dds"; // So check the filename extension (case insensitive)
	if (filename.size() >= 4 &&
		std::equal(dds.rbegin(), dds.rend(), filename.rbegin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }))
	{
		if(FAILED(DirectX::LoadFromDDSFile(ATL::CA2W(filename.c_str()), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage)))
		{
			throw std::runtime_error("Failed to load image: "+ filename);
		}
	}
	else
	{
		if(FAILED(DirectX::LoadFromWICFile(ATL::CA2W(filename.c_str()), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage)))
		{
			throw std::runtime_error("Failed to load image: " + filename);
		}
	}

	metadata.format = DirectX::MakeSRGB(metadata.format);


	D3D12_RESOURCE_DESC textureDesc = {};
	switch (metadata.dimension)
	{
	case DirectX::TEX_DIMENSION_TEXTURE1D:
		textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT16>(metadata.arraySize));
		break;
	case DirectX::TEX_DIMENSION_TEXTURE2D:
		textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.arraySize));
		break;
	case DirectX::TEX_DIMENSION_TEXTURE3D:
		textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.depth));
		break;
	default:
		throw std::exception("Invalid texture dimension.");
	}

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	mDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(textureResource.GetAddressOf()));

	return textureResource;
}

uint64_t CDX12Engine::ExecuteCommandList(ID3D12GraphicsCommandList2* commandList)
{
	commandList->Close();

	ID3D12CommandAllocator* commandAllocator;
	UINT                    dataSize = sizeof(commandAllocator);
	commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator);

	ID3D12CommandList* const ppCommandLists[] = {
		commandList
	};

	mCommandQueue->ExecuteCommandLists(1, ppCommandLists);
	uint64_t fenceValue = Signal();

	mCommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
	mCommandListQueue.push(commandList);

	// The ownership of the command allocator has been transferred to the ComPtr
	// in the command allocator queue. It is safe to release the reference 
	// in this temporary COM pointer here.
	commandAllocator->Release();

	return fenceValue;
}

void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent,
	std::chrono::milliseconds duration = std::chrono::milliseconds::max())
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		if (fence->SetEventOnCompletion(fenceValue, fenceEvent))
		{
			throw std::runtime_error("Error");
		}
		::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
	}
}

void CDX12Engine::Flush()
{
	uint64_t fenceValueForSignal = Signal();
	WaitForFenceValue(mFence, fenceValueForSignal, mFenceEvent);
}

void CDX12Engine::Resize(UINT width, UINT height)
{
	if (mWindow->GetWindowWidth() != width || mWindow->GetWindowHeight() != height)
	{
		// Don't allow 0 size swap chain back buffers.
		UINT newWidth = std::max(1u, width);
		UINT newHeight = std::max(1u, height);

		// Flush the GPU queue to make sure the swap chain's back buffers
		// are not being referenced by an in-flight command list.
		Flush();

		for (int i = 0; i < mNumFrames; ++i)
		{
			// Any references to the back buffers must be released
			// before the swap chain can be resized.
			mBackBuffers[i].Reset();
			mFrameFenceValues[i] = mFrameFenceValues[mCurrentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		if (SUCCEEDED(mSwapChain->GetDesc(&swapChainDesc)));
		if (SUCCEEDED(mSwapChain->ResizeBuffers(mNumFrames, newWidth, newHeight,
			swapChainDesc.BufferDesc.Format, swapChainDesc.Flags)));

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		mWindow->SetWindowSize(newWidth, newHeight);
		mMainScene->Resize(newWidth, newHeight);

		UpdateRenderTargetViews();
	}
}




void CDX12Engine::Finalize()
{
	auto commandAllocator = mCommandAllocators[mCurrentBackBufferIndex];
	auto backBuffer = mBackBuffers[mCurrentBackBufferIndex];

	commandAllocator->Reset();
	mCommandList->Reset(commandAllocator.Get(), nullptr);

	// Clear the render target.
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		mCommandList->ResourceBarrier(1, &barrier);

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrentBackBufferIndex, mRTVDescriptorSize);

		mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

	// Present
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mCommandList->ResourceBarrier(1, &barrier);

		if (mCommandList->Close())
		{

		}

		ID3D12CommandList* const commandLists[] = {
			mCommandList.Get()
		};
		mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		mFrameFenceValues[mCurrentBackBufferIndex] = Signal();

		UINT syncInterval = mMainScene->GetLockFps() ? 1 : 0;
		UINT presentFlags = !mMainScene->GetLockFps() ? DXGI_PRESENT_ALLOW_TEARING : 0;
		if (mSwapChain->Present(syncInterval, presentFlags))
		{
			throw std::runtime_error("Error");
		}

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		WaitForFenceValue(mFence, mFrameFenceValues[mCurrentBackBufferIndex], mFenceEvent);
	}
}



void CDX12Engine::LoadDefaultShaders()
{
	mDepthOnlyPixelShader.LoadShaderFromFile("Source/DX11/Shaders/DepthOnly_ps");
	mDepthOnlyNormalPixelShader.LoadShaderFromFile(("Source/DX11/Shaders/DepthOnlyNormal_ps"));
	mBasicTransformVertexShader.LoadShaderFromFile(("Source/DX11/Shaders/BasicTransform_vs"));
	mPbrVertexShader.LoadShaderFromFile(("Source/DX11/Shaders/PBRNoNormals_vs"));
	mPbrNormalVertexShader.LoadShaderFromFile(("Source/DX11/Shaders/PBR_vs"));
	mPbrPixelShader.LoadShaderFromFile(("Source/DX11/Shaders/PBRNoNormals_ps"));
	mPbrNormalPixelShader.LoadShaderFromFile(("Source/DX11/Shaders/PBR_ps"));
	mTintedTexturePixelShader.LoadShaderFromFile(("Source/DX11/Shaders/TintedTexture_ps"));
	mSkyPixelShader.LoadShaderFromFile(("Source/DX11/Shaders/Sky_ps"));
	mSkyVertexShader.LoadShaderFromFile(("Source/DX11/Shaders/Sky_vs"));
}

void CDX12Engine::InitD3D()
{
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
   // Using this awareness context allows the client area of the window 
   // to achieve 100% scaling while still allowing non-client window content to 
   // be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	EnableDebugLayer();

	ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(mUseWarp);

	mDevice = CreateDevice(dxgiAdapter4);

	CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

	mSwapChain = CreateSwapChain(mWindow->GetHandle(), mCommandQueue,
		mWindow->GetWindowWidth(), mWindow->GetWindowHeight(), mNumFrames);

	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, mNumFrames);
	mRTVDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	UpdateRenderTargetViews();

	for (int i = 0; i < mNumFrames; ++i)
	{
		mCommandAllocators[i] = CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	CreateCommandList(mCommandAllocators[mCurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

	mFence = CreateFence(mDevice);
	mFenceEvent = CreateEventHandle();
}
