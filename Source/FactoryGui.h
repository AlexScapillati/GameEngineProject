#pragma once

#include "IGUI.h"
#include "DX11/DX11Gui.h"


// Factory function, passing the engine the correct gui gets created
inline std::unique_ptr<IGui> NewGui(IEngine* engine)
{
	if (auto e = dynamic_cast<CDX11Engine*>(engine))
	{
		return std::make_unique<CDX11Gui>(e);
	}
	/*
	 if (auto e = dynamic_cast<CDX12Engine*>(engine))
	{
		return std::make_unique<CDX12Gui>(e);
	}
	*/
	return nullptr;
}
