
#pragma once

#include "Scene.h"
#include <Timer.h>

// Forward declarations of functions in this file
extern BOOL             InitWindow(HINSTANCE, int);
extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


class CDXEngine
{

public:

	CDXEngine(HINSTANCE hInstance, int nCmdShow);

	bool Update();

private:

	CScene* mMainScene;
	Timer mTimer;


};
