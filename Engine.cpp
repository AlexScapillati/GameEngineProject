#include "Engine.h"
#include "Direct3DSetup.h"

#include "External\imgui\imgui.h"
#include "External\imgui\imgui_impl_dx11.h"
#include "External\imgui\imgui_impl_win32.h"

void InitGui()
{
	//initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	io.ConfigDockingWithShift = false;

	// Setup Platform/Renderer bindings
	ImGui_ImplDX11_Init(gD3DDevice, gD3DContext);
	ImGui_ImplWin32_Init(gHWnd);
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}

extern void DisplayObjects();

void RenderGui()
{

	ImGui::Begin("Objects");

	DisplayObjects();

	ImGui::End();

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
			// Update the scene by the amount of time since the last frame
			const auto frameTime = mTimer.GetLapTime();
			mMainScene->UpdateScene(frameTime);

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			if(ImGui::BeginMainMenuBar())
			{
				ImGui::MenuItem("Open");
			}
			ImGui::EndMainMenuBar();

			//ImGui::Begin("Engine", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);
			//ImGui::SetWindowPos({ 0.0,0.0 });
			//ImGui::SetWindowSize({ (float)gViewportWidth, (float)gViewportHeight });


			ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse);

			// Draw the scene
			auto tex = mMainScene->RenderScene(frameTime);


			//render the scene image to ImGui
			ImGui::Image(tex, ImGui::GetWindowSize());

			ImGui::End();

			//render GUI
			RenderGui();

			//ImGui::End();

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
