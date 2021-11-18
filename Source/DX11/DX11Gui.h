#pragma once

#include <deque>

#include "DX11Engine.h"
#include "Objects/GameObject.h"


class CDX11Gui
{
public:
	
	CDX11Gui(CDX11Engine* engine);

	void Begin(float& frameTime);

	void Show(float& frameTime);

	void AddObjectsMenu() const;

	void DisplayPropertiesWindow() const;

	void DisplayObjects();

	void DisplaySceneSettings(bool& b) const;

	void DisplayShadowMaps() const;

	bool IsSceneFullscreen() const;

	template<class T>
	void DisplayDeque(std::deque<T*>& deque);

	~CDX11Gui();

private:
	CDX11Engine*              mEngine             = nullptr;
	CDX11Scene*               mScene              = nullptr;
	CDX11GameObject*          mSelectedObj        = nullptr;
	bool                      mViewportFullscreen = false;
	CVector2                  mViewportWindowPos  = {};
};
