#pragma once
#include "DX12GameObject.h"

class CDX12Sky : public CDX12GameObject
{
	public:

		void SetRotation(CVector3 rotation, int node) override;

		void Render(bool basicGeometry = false) override;
};

