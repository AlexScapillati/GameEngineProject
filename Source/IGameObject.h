#pragma once
#include "Utility/Input.h"

class IGameObject
{

public:

	// Render the object
	// Accepts a boolean value for rendering just the mesh 
	// Used in depth passes where the material does not need to be rendered
	virtual void Render(bool basicGeometry = false) = 0;

	virtual void Control(int node, float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
		KeyCode turnCW, KeyCode turnCCW, KeyCode moveForward, KeyCode moveBackward) = 0;

	// Virtual Destructor
	// Needed for every child classes
	//virtual ~IGameObject() = 0;

	virtual void Release() = 0;
};

