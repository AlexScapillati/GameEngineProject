//--------------------------------------------------------------------------------------
// GUI functions
//--------------------------------------------------------------------------------------

#pragma once

#include "Scene.h"
#include "Common.h"
#include "External\imgui\FileBrowser\ImGuiFileBrowser.h"
#include "External\imgui\imgui.h"
#include "External\imgui\ImGuizmo.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "External\imgui\imgui_impl_dx11.h"
#include "External\imgui\imgui_impl_win32.h"

#include "SpotLight.h"
#include "PointLight.h"
#include "DirLight.h"
#include "Light.h"

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

void ShutdownGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void DisplayShadowMaps()
{
	if (ImGui::Begin("ShadowMaps", 0, ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::BeginTable("", 6))
		{
			for (auto tx : GOM->mShadowsMaps)
			{
				ImTextureID texId = tx;

				ImGui::TableNextColumn();

				ImGui::Image((void*)texId, { 256, 256 });
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void CScene::AddObjectsMenu()
{
	static imgui_addons::ImGuiFileBrowser fileDialog;
	static bool addObj = false;

	enum EAddType
	{
		None,
		Simple,
		Pbr,
		SimpleLight,
		SpotLight,
		DirLight,
		OmniLight
	};

	static EAddType addType = None;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Simple Object"))
			{
				addType = Simple;
			}
			if (ImGui::MenuItem("Pbr Object"))
			{
				addType = Pbr;
			}
			if (ImGui::BeginMenu("Lights"))
			{
				if (ImGui::MenuItem("Simple Light"))
				{
					addType = SimpleLight;
				}
				if (ImGui::MenuItem("Spot Light"))
				{
					addType = SpotLight;
				}
				if (ImGui::MenuItem("Directional Light"))
				{
					addType = DirLight;
				}
				if (ImGui::MenuItem("Omnidirectional Light"))
				{
					addType = OmniLight;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	static bool selectMesh = false;
	static std::string mesh;
	static std::string tex;
	static std::string name;
	static std::string vs = mDefaultVs, ps = mDefaultPs;
	static bool selectTexture = false;
	static bool selectNormal = false;

	if (addType != None)
	{
		//open a window
		if (ImGui::Begin("Add Object"), &addObj, ImGuiWindowFlags_NoDocking)
		{
			//if the model is not pbr
			if (addType != Pbr)
			{
				//show a button that will open a file dialog to open a mesh
				if (ImGui::Button("Add Mesh"))
				{
					selectMesh = true;
				}

				//show the mesh file name
				if (mesh != "")
				{
					ImGui::SameLine();
					ImGui::Text(mesh.c_str());
				}

				//show a button that will open a file dialog to open a texture
				if (ImGui::Button("Add Texture"))
				{
					selectTexture = true;
				}
			}
			else
			{
				//if the model is pbr just show a button for opening a file dialog to open a mesh file that contains the id of the model
				if (ImGui::Button("Add ID"))
				{
					selectMesh = true;
				}
				//show the mesh file name
				if (mesh != "")
				{
					ImGui::SameLine();
					ImGui::Text(mesh.c_str());
				}
			}

			//show the texture filename
			if (tex != "")
			{
				ImGui::SameLine();
				ImGui::Text(tex.c_str());
			}

			// colour button for the light objects
			static CVector3 col;
			static float strenght;

			//if the model is a light (hence not a simple object or pbr)
			if (addType != Simple && addType != Pbr)
			{
				ImGui::ColorEdit3("LightColor", col.GetValuesArray());
				ImGui::DragFloat("Strength", &strenght);
			}

			//show button that will add the object in the object manager
			if (ImGui::Button("Add"))
			{
				addObj = false;

				//get the actual name without the extension
				auto pos = mesh.find('.');

				name = mesh.substr(0, pos);

				//depending on the type create the corresponding object type

				switch (addType)
				{
				case Simple:

					if (mesh != "" && tex != "")
					{
						auto newObj = new CGameObject(mesh, name, tex, vs, ps, { 0,0,0 }, { 0,0,0 }, 1);

						mObjManager->AddObject(newObj);

						addObj = false;
					}

					break;

				case Pbr:

					if (mesh != "")
					{
						auto pos = mesh.find_first_of('_');

						auto id = mesh.substr(0, pos);

						auto newObj = new CGameObject(id, name, vs, ps, { 0,0,0 }, { 0,0,0 }, 1);

						mObjManager->AddObject(newObj);

						addObj = false;
					}

					break;

				case SimpleLight:

					if (mesh != "" & tex != "")
					{
						auto newObj = new CLight(mesh, name, tex, vs, ps, col, strenght);

						mObjManager->AddLight(newObj);

						addObj = false;
					}

					break;

				case SpotLight:

					if (mesh != "" & tex != "")
					{
						auto newObj = new CSpotLight(mesh, name, tex, vs, ps, col, strenght);

						mObjManager->AddSpotLight(newObj);

						addObj = false;
					}

					break;

				case DirLight:

					if (mesh != "" & tex != "")
					{
						auto newObj = new CDirLight(mesh, name, tex, vs, ps, col, strenght);

						mObjManager->AddDirLight(newObj);

						addObj = false;
					}

					break;

				case OmniLight:

					if (mesh != "" & tex != "")
					{
						auto newObj = new CPointLight(mesh, name, tex, vs, ps, col, strenght);

						mObjManager->AddPointLight(newObj);

						addObj = false;
					}

					break;
				}
			}
		}
		ImGui::End();
	}

	//open file dialog code
	if (selectMesh)
	{
		ImGui::OpenPopup("Select Mesh");
	}
	if (selectTexture)
	{
		ImGui::OpenPopup("Select Texture");
	}

	if (fileDialog.showFileDialog("Select Mesh", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".x,.fbx"))
	{
		selectMesh = false;
		mesh = fileDialog.selected_fn;
	}

	if (fileDialog.showFileDialog("Select Texture", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".jpg,.dds,.png"))
	{
		selectTexture = false;
		tex = fileDialog.selected_fn;
	}
}

std::string ChooseTexture(bool& selected, imgui_addons::ImGuiFileBrowser fileDialog)
{
	if (fileDialog.showFileDialog("Select Texture", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".jpg,.dds,.png"))
	{
		selected = false;
		return fileDialog.selected_fn;
	}
}

template <class T>
void CScene::DisplayDeque(std::deque<T*> deque)
{
	auto it = deque.begin();

	while (it != deque.end())
	{
		//if a button is pressed
		if (ImGui::Button((*it)->GetName().c_str()))
		{
			//store the object pointer
			mSelectedObj = *it;
		}

		ImGui::SameLine();

		auto deleteLabel = "Delete##" + (*it)->GetName();

		//draw a button on the same line to delete the current object
		if (ImGui::Button(deleteLabel.c_str()))
		{
			//delete the current iterator from the container
			it = deque.erase(it);
		}
		else
		{
			//increment the iterator
			it++;
		}
	}
}

void CScene::DisplayPropertiesWindow()
{
	static auto mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	static bool showBounds = false;

	static bool show = !(mSelectedObj == nullptr);
	if (ImGui::Begin("Properties", &show))
	{
		ImGuizmo::Enable(mSelectedObj->Enabled());

		ImGui::Separator();

		if (ImGui::Button("Duplicate Selected Obj"))
		{
			//WIP
			try
			{
				if (auto light = dynamic_cast<CLight*>(mSelectedObj))
				{
					if (auto spotLight = dynamic_cast<CSpotLight*>(mSelectedObj))
					{
						auto obj = new CSpotLight(*spotLight);

						GOM->AddSpotLight(obj);
					}
					else if (auto dirLight = dynamic_cast<CDirLight*>(mSelectedObj))
					{
						auto obj = new CDirLight(*dirLight);

						GOM->AddDirLight(obj);
					}
					else if (auto omniLight = dynamic_cast<CPointLight*>(mSelectedObj))
					{
						auto obj = new CPointLight(*omniLight);

						GOM->AddPointLight(obj);
					}
					else
					{
						auto obj = new CLight(*light);

						GOM->AddLight(obj);
					}
				}
				else
				{
					auto obj = new CGameObject(*mSelectedObj);

					GOM->AddObject(obj);
				}
			}
			catch (std::exception& e)
			{
				throw std::runtime_error(e.what());
			}
		}

		//auto name = ' ';

		//if (ImGui::InputText("Name", &name, IM_ARRAYSIZE(&name)))
		//{
		//	selectedObj->SetName(std::to_string(name));
		//}

		ImGui::Checkbox("Enabled", mSelectedObj->Enabled());

		ImGui::Checkbox("Toggle ambient Map", mSelectedObj->AmbientMapEnabled());

		//display the transform component
		ImGui::NewLine();
		ImGui::Separator();

		ImGui::Text("Transform");

		if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
			mCurrentGizmoOperation = ImGuizmo::SCALE;

		//get the direct access to the position of the model and display it
		ImGui::DragFloat3("Position", mSelectedObj->DirectPosition());

		//acquire the rotation array
		float* rot = mSelectedObj->Rotation().GetValuesArray();

		//convert it to degreese
		for (int i = 0; i < 3; i++)
		{
			rot[i] = ToDegrees(rot[i]);
		}

		//display the rotation
		if (ImGui::DragFloat3("Rotation", rot))
		{
			//if the value is changed
			//get back to radians
			for (int i = 0; i < 3; i++)
			{
				rot[i] = ToRadians(rot[i]);
			}

			//set the rotation
			mSelectedObj->SetRotation(rot);
		}

		//get the scale array
		float* scale = mSelectedObj->Scale().GetValuesArray();

		//display the scale array
		if (ImGui::DragFloat3("Scale", scale, 0.1f, 0.001f, D3D11_FLOAT32_MAX))
		{
			//if it has changed set the scale
			mSelectedObj->SetScale(scale);
		}

		ImGui::Checkbox("Show Bounds", &showBounds);

		//----------------------------------------------------------------
		// Object Specific settings
		//----------------------------------------------------------------

		if (auto light = dynamic_cast<CLight*>(mSelectedObj))
		{
			ImGui::Text("Specific settings");

			//light colour edit
			static bool colourPickerOpen = false;

			if (ImGui::Button("Edit Colour"))
			{
				colourPickerOpen = !colourPickerOpen;
			}

			if (colourPickerOpen)
			{
				if (ImGui::Begin("ColourPicker", &colourPickerOpen))
				{
					ImGui::ColorPicker3("Colour", light->GetColour().GetValuesArray());
				}
				ImGui::End();
			}

			//modify strength
			ImGui::DragFloat("Strength", &light->GetStrength(), 1.0f, 0.0f, D3D11_FLOAT32_MAX);

			//if it is a spotlight let it modify few things
			if (auto spotLight = dynamic_cast<CSpotLight*>(mSelectedObj))
			{
				//modify facing
				CVector3 facingV = spotLight->GetFacing();

				if (ImGui::DragFloat3("Facing", facingV.GetValuesArray(), 0.001f, -1.0f, 1.0f))
				{
					CVector3 facing = Normalise(facingV);
					spotLight->SetFacing(facing);
				}

				//modify cone angle
				ImGui::DragFloat("Cone Angle", &spotLight->GetConeAngle(), 1.0f, 0.0f, 180.0f);

				//modify shadow map size
				ImGui::DragInt("ShadowMapsSize", &spotLight->GetShadowMapSize(), 1.0f, 2, 16384);
			}
			else if (auto dirLight = dynamic_cast<CDirLight*>(mSelectedObj))
			{
				//modify direction

				auto facingV = dirLight->GetDirection();

				if (ImGui::DragFloat3("Facing", facingV.GetValuesArray(), 0.001f, -1.0f, 1.0f))
				{
					CVector3 facing = Normalise(facingV);
					dirLight->SetDirection(facing);
				}
				//modify shadow map size

				auto size = dirLight->GetShadowMapSize();

				if (ImGui::DragInt("ShadowMapsSize", &size, 1.0f, 2, 16384))
				{
					if (size < 16384 && size > 2)
					{
						dirLight->SetShadowMapSize(size);
					}
					else
					{
						std::runtime_error("Number Too big!");
					}
				}

				//modify near clip and far clip

				auto nearClip = dirLight->GetNearClip();
				auto farClip = dirLight->GetFarClip();

				if (ImGui::DragFloat("NearClip", &nearClip, 0.01f, 0.0f, 10.0f))
				{
					dirLight->SetNearClip(nearClip);
				}

				if (ImGui::DragFloat("FarClip", &farClip, 10.0f, 0.0f, D3D11_FLOAT32_MAX))
				{
					dirLight->SetFarClip(farClip);
				}
			}
		}

		//display the texture
		ImGui::NewLine();
		ImGui::Text("Texture");

		ImTextureID texId = mSelectedObj->GetTextureSRV();

		ImGui::Image((void*)texId, { 256,256 });

		//display the ambient map (if any)
		if (*mSelectedObj->AmbientMapEnabled())
		{
			ImGui::NewLine();
			ImGui::Text("AmbientMap");

			//ImGui::Image((void*)selectedObj->GetAmbientMap(), {256.f, 256.f});
		}
	}
	ImGui::End();

	auto pos = gViewportWindowPos;

	ImGuizmo::SetRect(pos.x, pos.y, mViewportX, mViewportY);

	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	static float bounds[] =
	{
		0.0f,0.0f,0.0f,
		 1.f, 1.f, 1.f
	};

	ImGuizmo::Manipulate(mCamera->ViewMatrix().GetArray(), mCamera->ProjectionMatrix().GetArray(),
		mCurrentGizmoOperation, ImGuizmo::WORLD, mSelectedObj->WorldMatrix().GetArray(), 0, 0, showBounds ? bounds : 0);
}

void CScene::DisplayObjects()
{
	if (ImGui::Begin("Objects", 0, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar))
	{
		AddObjectsMenu();

		ImGui::Separator();

		//display for each model a button

		DisplayDeque(GOM->mObjects);
		DisplayDeque(GOM->mLights);
		DisplayDeque(GOM->mSpotLights);
		DisplayDeque(GOM->mDirLights);
		DisplayDeque(GOM->mPointLights);
	}
	ImGui::End();

	if (mSelectedObj != nullptr)
	{
		DisplayPropertiesWindow();
	}
}