#pragma once
#include <deque>

#include "GUI.h"


class CDX11Gui final : public IGui
{
public:
	
	CDX11Gui(CDX11Engine* engine);

	void Begin(float& frameTime) override;

	void Show(float& frameTime) override;

	void AddObjectsMenu() const;

	void DisplayPropertiesWindow() const;

	void DisplayObjects();

	void DisplaySceneSettings(bool& b) const;

	void DisplayShadowMaps() const;


	template<class T>
	void DisplayDeque(std::deque<T*>& deque);
	

	~CDX11Gui() override;

private:

	CDX11Engine* mEngine;

	IScene* mScene;

	CGameObject* mSelectedObj = nullptr;
};
