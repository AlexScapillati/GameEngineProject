//--------------------------------------------------------------------------------------
// GUI functions
//--------------------------------------------------------------------------------------

#pragma once

#include "FactoryEngine.h"
#include <CVector2.h>

class CGameObject;
class IScene;

class IGui
{

public:
	virtual ~IGui() = default;

	virtual void Show(float& frameTime) = 0;

	bool IsSceneFullscreen() const
	{
		return mViewportFullscreen;
	}

	virtual void Begin(float& frameTime) = 0;

protected:


	bool mViewportFullscreen = false;

	CVector2 mViewportWindowPos;
	
};

