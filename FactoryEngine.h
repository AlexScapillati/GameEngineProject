#pragma once

#include "Engine.h"
#include "DX11Engine.h"
//#include "DX12Engine.h"



// Factory function that creates the correct engine

enum EApiType
{
	DX11,
	DX12
};

inline std::unique_ptr<IEngine> NewEngine(EApiType type, HINSTANCE hInstance, int nCmdShow)
{
	switch (type)
	{
	case DX11:
		return std::make_unique<CDX11Engine>(hInstance, nCmdShow);
	case DX12:
		//return std::make_unique<CDX12Engine>(hInstance, nCmdShow);
		return nullptr;
	default:
		return nullptr;
	}
}