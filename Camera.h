//--------------------------------------------------------------------------------------
// Class encapsulating a camera
//--------------------------------------------------------------------------------------
// Holds position, rotation, near/far clip and field of view. These to a view and projection matrices as required

#pragma once

#include "CVector3.h"
#include "CVector2.h"
#include "CMatrix4x4.h"
#include "MathHelpers.h"
#include "Input.h"



class CCamera
{
public:
	//-------------------------------------
	// Construction and Usage
	//-------------------------------------

	// Constructor - initialise all settings, sensible defaults provided for everything.
	CCamera(CVector3 position = {0,0,0}, CVector3 rotation = {0,0,0}, 
           float fov = PI/3, float aspectRatio = 16.0f / 9.0f, float nearClip = 0.1f, float farClip = 10000.0f)
        : mPosition(position), mRotation(rotation), mFOVx(fov), mAspectRatio(aspectRatio), mNearClip(nearClip), mFarClip(farClip)
    {
    }


	// Control the camera's position and rotation using keys provided
	void Control( float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,  
	              KeyCode moveForward, KeyCode moveBackward, KeyCode moveLeft, KeyCode moveRight);

	void SetAspectRatio(float r);


	//-------------------------------------
	// Data access
	//-------------------------------------

	// Getters / setters
	CVector3 Position() const { return mPosition; }
	CVector3 Rotation() const { return mRotation;	}
	void SetPosition(CVector3 position)  { mPosition = position; }
	void SetRotation(CVector3 rotation)  { mRotation = rotation; }

	float FOV() const { return mFOVx;     }
	float NearClip() const { return mNearClip; }
	float FarClip() const { return mFarClip;  }

	void SetFOV     (float fov     )  { mFOVx     = fov;      }
	void SetNearClip(float nearClip)  { mNearClip = nearClip; }
	void SetFarClip (float farClip )  { mFarClip  = farClip;  }

	// Read only access to camera matrices, updated on request from position, rotation and camera settings
	CMatrix4x4& WorldMatrix()           { UpdateMatrices(); return mWorldMatrix; }
	CMatrix4x4& ViewMatrix()            { UpdateMatrices(); return mViewMatrix;           }
	CMatrix4x4& ProjectionMatrix()      { UpdateMatrices(); return mProjectionMatrix;     }
	CMatrix4x4& ViewProjectionMatrix()  { UpdateMatrices(); return mViewProjectionMatrix; }
	
	//-------------------------------------
	// Camera picking
	//-------------------------------------

	CVector3 PixelFromWorldPt(CVector3 worldPoint, unsigned int viewportWidth, unsigned int viewportHeight);

	CVector2 PixelSizeInWorldSpace(float Z, unsigned int viewportWidth, unsigned int viewportHeight);

	
//-------------------------------------
// Private members
//-------------------------------------
private:
	// Update the matrices used for the camera in the rendering pipeline
	void UpdateMatrices();

	// Postition and rotations for the camera (rarely scale cameras)
	CVector3 mPosition;
	CVector3 mRotation;

	// Camera settings: field of view, aspect ratio, near and far clip plane distances.
	// Note that the FOVx angle is measured in radians (radians = degrees * PI/180) from left to right of screen
	float mFOVx;
    float mAspectRatio;
	float mNearClip;
	float mFarClip;

	// Current view, projection and combined view-projection matrices (DirectX matrix type)
	CMatrix4x4 mWorldMatrix; // Easiest to treat the camera like a model and give it a "world" matrix...
	CMatrix4x4 mViewMatrix;  // ...then the view matrix used in the shaders is the inverse of its world matrix

	CMatrix4x4 mProjectionMatrix;     // Projection matrix holds the field of view and near/far clip distances
	CMatrix4x4 mViewProjectionMatrix; // Combine (multiply) the view and projection matrices together, which
	                                  // can sometimes save a matrix multiply in the shader (optional)
};

