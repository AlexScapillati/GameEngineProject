
#include "Window.h"
#include <intsafe.h>
#include <stdexcept>

#include "Common.h"

UINT CWindow::GetWindowWidth()
{
	return mWindowWidth;
}

UINT CWindow::GetWindowHeight()
{
	return mWindowHeight;
}

void CWindow::SetWindowSize(UINT x, UINT y)
{
	mWindowHeight = y;
	mWindowWidth = x;
}

HWND CWindow::GetHandle()
{
	return mHWnd;
}

CWindow::~CWindow()
{
	DestroyWindow(mHWnd);
}

extern LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------------------------
// Create a window to display our scene, returns false on failure.
//--------------------------------------------------------------------------------------
CWindow::CWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Get a stock icon to show on the taskbar for this program.
	SHSTOCKICONINFO stockIcon;
	stockIcon.cbSize = sizeof(stockIcon);
	if (SHGetStockIconInfo(SIID_APPLICATION, SHGSI_ICON, &stockIcon) != S_OK) // Returns false on failure
	{
		throw std::runtime_error("Impossible get the stock icon");
	}

	// Register window class. Defines various UI features of the window for our application.
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;    // Which function deals with windows messages
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0; SIID_APPLICATION;
	wcex.hInstance = hInstance;
	wcex.hIcon = stockIcon.hIcon; // Which icon to use for the window
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW); // What cursor appears over the window
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"CO3303WindowClass";
	wcex.hIconSm = stockIcon.hIcon;
	if (!RegisterClassEx(&wcex)) // Returns false on failure
	{
		throw std::runtime_error("Impossible initialize window");
	}

	// Select the type of window to show our application in
	const DWORD windowStyle = WS_OVERLAPPEDWINDOW; // Standard window
	//DWORD windowStyle = WS_POPUP;          // Alternative: borderless. If you also set the viewport size to the monitor resolution, you
											 // get a "fullscreen borderless" window, which works better with alt-tab than DirectX fullscreen,
											 // which is an option in Direct3DSetup.cpp

	// Calculate overall dimensions for the window. We will render to the *inside* of the window. But the
	// overall winder will be larger if it includes borders, title bar etc. This code calculates the overall
	// size of the window given our choice of viewport size.
	RECT rc = { 0, 0, (LONG)mWindowWidth, (LONG)mWindowHeight };
	AdjustWindowRect(&rc, windowStyle, FALSE);

	// Create window, the second parameter is the text that appears in the title bar at first
	//gHInst = hInstance;
	mHWnd = CreateWindow(L"CO3303WindowClass", L"Direct3D 11", windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	if (!mHWnd)
	{
		throw std::runtime_error("Impossible create window");
	}

	ShowWindow(mHWnd, nCmdShow);
	UpdateWindow(mHWnd);
}

