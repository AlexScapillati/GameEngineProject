//--------------------------------------------------------------------------------------
// GUI functions
//--------------------------------------------------------------------------------------

#pragma once

#include "FactoryEngine.h"

class CDX11GameObject;
class CDX11Scene;

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

