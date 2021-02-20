#include "Engine.h"
#include "Direct3DSetup.h"

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

			// Draw the scene
			mMainScene->RenderScene(frameTime);

			//render the scene image to ImGui


			if (KeyHit(Key_Escape))
			{
				DestroyWindow(gHWnd); // This will close the window and ultimately exit this loop
			}
		}
	}

	ShutdownDirect3D();

	return (int)msg.wParam;
}
