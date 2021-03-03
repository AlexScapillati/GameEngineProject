
#pragma once

#include "Scene.h"
#include <Timer.h>

// Forward declarations of functions in this file
extern BOOL             InitWindow(HINSTANCE, int);
extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


class CDXEngine
{

public:

	void RenderGui();

	CDXEngine(HINSTANCE hInstance, int nCmdShow);

	bool Update();

	~CDXEngine();

private:

	std::unique_ptr<CScene> mMainScene;
	Timer mTimer;


};
