#pragma once

#include <wrl.h>
#include <string>
#include <memory>

#include "Utility/Timer.h"

class IScene;
using namespace Microsoft::WRL;

// Forward declarations

class CDX11Scene;
class CWindow;
class IGui;

class IEngine
{
public:


	//*******************************
	//**** Virtual Functions 
	 
	virtual bool Update() = 0;

	virtual void Resize(UINT x, UINT y) = 0;

	virtual void Finalize() = 0;

	//*******************************
	//**** Setters / Getters

	auto GetTimer() const
	{
		return mTimer;
	}

	auto GetWindow() const
	{
		return mWindow.get();
	}

	auto GetScene() const
	{
		return mMainScene.get();
	}

	auto& GetMediaFolder()
	{
		return mMediaFolder;
	}

	auto GetGui() const
	{
		return mGui.get();
	}


	virtual ~IEngine() = default;

protected:

	//*******************************
	//**** Data


	std::unique_ptr<CDX11Scene> mMainScene;

	Timer mTimer;

	std::unique_ptr<CWindow> mWindow;

	std::unique_ptr<IGui> mGui;

	std::string mMediaFolder;

	std::string mShaderFolder;

	std::string mPostProcessingFolder;
};
