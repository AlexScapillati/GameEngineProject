#pragma once

#include <Timer.h>
#include <wrl.h>
#include <string>
#include <memory>

using namespace Microsoft::WRL;

// Forward declarations

class IScene;
class CWindow;
class IGui;

class IEngine
{
public:

	virtual bool Update() = 0;

	virtual void Resize(UINT x, UINT y) = 0;

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

	virtual void Finalize() = 0;

	virtual ~IEngine() = default;

protected:

	std::unique_ptr<IScene> mMainScene;

	Timer mTimer;

	std::unique_ptr<CWindow> mWindow;

	std::unique_ptr<IGui> mGui;

	std::string mMediaFolder;
};
