#pragma once

#include <wrl.h>
#include <string>
#include <memory>

#include "Utility/Timer.h"

// Forward declarations

class CWindow;

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


	auto& GetMediaFolder()
	{
		return mMediaFolder;
	}
	
	virtual ~IEngine() = default;

protected:

	//*******************************
	//**** Data

	Timer mTimer;

	std::unique_ptr<CWindow> mWindow;

	std::string mMediaFolder;

	std::string mShaderFolder;

	std::string mPostProcessingFolder;
};
