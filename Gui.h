#pragma once

#include "External\imgui\imgui.h"
#include "External\imgui\imgui_impl_dx11.h"
#include "External\imgui\imgui_impl_win32.h"
#include "Common.h"

#include <vector>
#include "External\imgui\imgui_internal.h"
#include "Scene.h"

class CGui
{
public:

	CGui()
	{
		//initialize ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

			// Setup Platform/Renderer bindings
		ImGui_ImplDX11_Init(gD3DDevice, gD3DContext);
		ImGui_ImplWin32_Init(gHWnd);
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
	}

	void Render()
	{

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		for (auto window : mWindows)
		{

		}

	}


	void AddWindow(ImGuiWindow* window)
	{
		mWindows.push_back(window);
	}

	~CGui()
	{
		for (auto window : mWindows)
		{
			delete window;
		}

		mWindows.clear();

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

private:

	std::vector<ImGuiWindow*> mWindows;
};
