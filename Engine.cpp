#include "Engine.h"
#include "Direct3DSetup.h"

#include "External\imgui\imgui.h"
#include "External\imgui\imgui_impl_dx11.h"
#include "External\imgui\imgui_impl_win32.h"
#include "External\imgui\ImGuizmo.h"

void InitGui()
{
	//initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows //super broken

	io.ConfigDockingWithShift = false;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	// Setup Platform/Renderer bindings
	ImGui_ImplDX11_Init(gD3DDevice, gD3DContext);
	ImGui_ImplWin32_Init(gHWnd);
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}


void CDXEngine::RenderGui()
{

	mMainScene->DisplayObjects();

}

void ShutdownGui()
{

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

}

CDXEngine::CDXEngine(HINSTANCE hInstance, int nCmdShow)
{
	// Create a window to display the scene
	if (!InitWindow(hInstance, nCmdShow))
	{
		throw std::runtime_error("Impossible initialize window");
	}

	//get the executable path

	CHAR path[MAX_PATH];

	GetModuleFileNameA(hInstance, path, MAX_PATH);

	const auto pos = std::string(path).find_last_of("\\/");

	//get the media folder
	gMediaFolder = std::string(path).substr(0, pos) + "/Media/";

	// Prepare TL-Engine style input functions
	InitInput();

	// Initialise Direct3D
	if (!InitDirect3D())
	{

		ShutdownDirect3D();

		throw std::runtime_error("Impossible initialize DirectX");
	}

	//create gui

	//auto gGui = std::make_unique<CGui>();

	InitGui();

	try
	{
		mMainScene = std::make_unique<CScene>("Scene1.xml");
	}
	catch (std::exception e)
	{
		ShutdownDirect3D();

		throw std::runtime_error(e.what());
	}

	// Will use a timer class to help in this tutorial (not part of DirectX). It's like a stopwatch - start it counting now
	mTimer.Start();
}

bool CDXEngine::Update()
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
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			if (ImGui::BeginMainMenuBar())
			{
				ImGui::MenuItem("Open");


				ImGui::EndMainMenuBar();
			}

			auto vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos({ 0,0 });
			ImGui::SetNextWindowSize({ (float)gViewportWidth,(float)gViewportHeight });

			if (ImGui::Begin("Engine"))
			{
				if (ImGui::Begin("Viewport", nullptr,
					ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_MenuBar))
				{
					// Update the scene by the amount of time since the last frame
					const auto frameTime = mTimer.GetLapTime();
					mMainScene->UpdateScene(frameTime);

					if (ImGui::BeginMenuBar())
					{
						ImGui::MenuItem("Maximize", "", &gViewportFullscreen);
						ImGui::EndMenuBar();
					}

					//get the available region of the window 
					auto size = ImGui::GetContentRegionAvail();

					if (gViewportFullscreen)
					{
						size = { (float)gViewportWidth, (float)gViewportHeight };
						ImGui::SetWindowSize(size);
					}

					//compare it with the scene viewport
					if ((size.x != mMainScene->mViewportX || size.y != mMainScene->mViewportY) && (size.x != 0 && size.y != 0))
					{
						//if they are different, resize the scene viewport
						mMainScene->Resize(size.x, size.y);
					}

					// Draw the scene
					auto sceneTexture = mMainScene->RenderScene(frameTime);

					// Set the back buffer as the target for rendering and select the main depth buffer.
					// When finished the back buffer is sent to the "front buffer" - which is the monitor.
					gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

					// Clear the back buffer to a fixed colour and the depth buffer to the far distance
					gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &mMainScene->gBackgroundColor.r);
					gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

					//render the scene image to ImGui
					ImGui::Image(sceneTexture, size);
				}
				ImGui::End();
			}
			ImGui::End();

			//render GUI
			if (!gViewportFullscreen)
				RenderGui();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			ImGui::EndFrame();
			ImGui::UpdatePlatformWindows();

			////--------------- Scene completion ---------------////

			// When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
			// Set first parameter to 1 to lock to vsync
			if (gSwapChain->Present(mMainScene->mLockFPS ? 1 : 0, 0) == DXGI_ERROR_DEVICE_REMOVED)
			{
				gD3DDevice->GetDeviceRemovedReason();

				throw std::runtime_error("Device Removed");
			}

			if (KeyHit(Key_Escape))
			{
				DestroyWindow(gHWnd); // This will close the window and ultimately exit this loop
			}
		}
	}

	ShutdownGui();

	ShutdownDirect3D();

	return (int)msg.wParam;
}
