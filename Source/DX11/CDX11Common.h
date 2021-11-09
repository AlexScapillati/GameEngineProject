//--------------------------------------------------------------------------------------
// Commonly used definitions across entire project
//--------------------------------------------------------------------------------------

#pragma once

#include "..\Math\CVector2.h"
#include "..\Math\CVector3.h"
#include "..\Math\CVector4.h"
#include "..\Math\CMatrix4x4.h"

#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;


class CGameObjectManager;

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global Variables from various files available to other files. "extern" means
// this variable is defined in another file somewhere. We should use classes and avoid
// use of globals, but done this way to keep code simpler so the DirectX content is
// clearer. However, try to architect your own code in a better way.

// Input constants
extern float ROTATION_SPEED;
extern float MOVEMENT_SPEED;

//--------------------------------------------------------------------------------------
// Light Structures
//--------------------------------------------------------------------------------------

struct sLight
{
	CVector3 position;
	float enabled;
	CVector3 colour;
	float intensity;
};

struct sSpotLight
{
	CVector3 colour;
	float enabled;
	CVector3 pos;
	float intensity;
	CVector3 facing;          //the direction facing of the light
	float cosHalfAngle;       //pre calculate this in the c++ side, for performance reasons
	CMatrix4x4 viewMatrix;    //the light view matrix (as it was a camera)
	CMatrix4x4 projMatrix;    //--"--
};

struct sDirLights
{
	CVector3 colour;
	float enabled;
	CVector3 facing;
	float intensity;
	CMatrix4x4 viewMatrix;    //the light view matrix (as it was a camera)
	CMatrix4x4 projMatrix;    //--"--
};

struct sPointLights
{
	CVector3 colour;
	float enabled;
	CVector3 position;
	float intensity;
	CMatrix4x4 viewMatrices[6];    //the light view matrix (as it was a camera)
	CMatrix4x4 projMatrix;    //--"--
};

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame

const int MAX_LIGHTS = 64;

// Data that remains constant for an entire frame, updated from C++ to the GPU shaders *once per frame*
// We hold them together in a structure and send the whole thing to a "constant buffer" on the GPU each frame when
// we have finished updating the scene. There is a structure in the shader code that exactly matches this one
struct PerFrameConstants
{
	// These are the matrices used to position the camera
	CMatrix4x4 cameraMatrix;
	CMatrix4x4 viewMatrix;
	CMatrix4x4 projectionMatrix;
	CMatrix4x4 viewProjectionMatrix; // The above two matrices multiplied together to combine their effects

	CVector3   ambientColour;
	float      specularPower;

	float		parallaxMinSamples = 5;
	float		parallaxMaxSamples = 20;
	float	parallaxPadding;

	float		gDepthAdjust = 0.00005f;

	float		nLights;
	float		nDirLight;
	float		nSpotLights;
	float		nPointLights;

	uint32_t	nPcfSamples;
	CVector3	padding2;

	CVector3   cameraPosition;
	float      frameTime;      // This app does updates on the GPU so we pass over the frame update time
};

extern PerFrameConstants gPerFrameConstants;      // This variable holds the CPU-side constant buffer described above
extern ComPtr < ID3D11Buffer> gPerFrameConstantBuffer; // This variable controls the GPU-side constant buffer matching to the above structure

struct PerFrameLights
{
	sLight lights[MAX_LIGHTS];
};

extern PerFrameLights gPerFrameLightsConstants;
extern ComPtr < ID3D11Buffer> gPerFrameLightsConstBuffer;

struct PerFrameSpotLights
{
	sSpotLight spotLights[MAX_LIGHTS];
};

extern PerFrameSpotLights gPerFrameSpotLightsConstants;
extern ComPtr < ID3D11Buffer> gPerFrameSpotLightsConstBuffer;

struct PerFrameDirLights
{
	sDirLights dirLights[MAX_LIGHTS];
};

extern PerFrameDirLights gPerFrameDirLightsConstants;
extern ComPtr < ID3D11Buffer> gPerFrameDirLightsConstBuffer;

struct PerFramePointLights
{
	sPointLights pointLights[MAX_LIGHTS];
};

extern PerFramePointLights gPerFramePointLightsConstants;
extern ComPtr < ID3D11Buffer> gPerFramePointLightsConstBuffer;

static const int MAX_BONES = 64;

// This is the matrix that positions the next thing to be rendered in the scene. Unlike the structure above this data can be
// updated and sent to the GPU several times every frame (once per model). However, apart from that it works in the same way.
struct PerModelConstants
{
	CMatrix4x4 worldMatrix;

	CVector3   objectColour;  // Allows each light model to be tinted to match the light colour they cast
	float      parallaxDepth; // Used in the geometry shader to control how much the polygons are exploded outwards

	float		hasOpacityMap;
	float		hasAoMap;
	float		hasRoughnessMap;
	float		hasAmbientMap;
	float		hasMetallnessMap;

	float       roughness;
	float		metalness;
	float		padding;

	CMatrix4x4 boneMatrices[MAX_BONES];
};

extern PerModelConstants gPerModelConstants;      // This variable holds the CPU-side constant buffer described above
extern ComPtr<ID3D11Buffer> gPerModelConstantBuffer; // This variable controls the GPU-side constant buffer related to the above structure



// Settings used by post-processes - must match the similar structure in the Common.hlsli shader file
struct PostProcessingConstants
{
	CVector2 area2DTopLeft; // Top-left of post-process area on screen, provided as coordinate from 0.0->1.0 not as a pixel coordinate
	CVector2 area2DSize; // Size of post-process area on screen, provided as sizes from 0.0->1.0 (1 = full screen) not as a size in pixels
	float area2DDepth; // Depth buffer value for area (0.0 nearest to 1.0 furthest). Full screen post-processing uses 0.0f
	CVector3 paddingA; // Pad things to collections of 4 floats (see notes in earlier labs to read about padding)

	CVector4 polygon2DPoints[4]; // Four points of a polygon in 2D viewport space for polygon post-processing. Matrix transformations already done on C++ side

	// Tint post-process settings
	CVector3 tintColour = { 1,1,1 };

	// Grey noise post-process settings
	float noiseStrength = 0.5;
	CVector2 noiseScale;
	CVector2 noiseOffset;
	float noiseEdge;
	CVector3 paddingB;

	// Burn post-process settings
	float burnHeight;
	CVector3 paddingC;

	// Distort post-process settings
	float distortLevel = 0.03f; //distorsion level for the dirstorsion effect
	CVector3 paddingD;

	// Spiral post-process settings
	float spiralLevel;
	CVector3 paddingE;

	// Heat haze post-process settings
	float heatHazeTimer;
	float heatEffectStrength = 0.005f;
	float heatSoftEdge = 0.125f;// Softness of the edge of the circle - range 0.001 (hard edge) to 0.25 (very soft)
	float paddingF;

	// Chromatic Aberration settings
	float caAmount = 0.01f;
	float   caEdge = 0.5f;
	float   caFalloff = 0.01f;
	float paddingG;

	//gaussian blur settings
	float blurDirections = 16; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
	float blurQuality = 3.0; // BLUR QUALITY (Default 4.0 - More is better but slower)
	float blurSize = 8.0; // BLUR SIZE (Radius)
	float paddingH;

	// Bloom settings
	float bloomThreshold = 0.5f;
	CVector3 paddingI;

	// SSAO settings
	float ssaoStrenght = 1.0f;
	float ssaoArea = 0.2f;
	float ssaoFalloff = 0.000001f;
	float ssaoRadius = 0.1f;

	// God Rays Settings
	CVector2 lightScreenPos;
	float	 weight = 0.58767f;
	float	 decay = 0.97815f;
	float	 exposure = 0.2f;
	float	 density = 0.926f;
	int		 numSamples = 4;
	float	 paddingJ;
};
extern PostProcessingConstants gPostProcessingConstants; // This variable holds the CPU-side constant buffer described above
extern ComPtr<ID3D11Buffer> gPostProcessingConstBuffer; // This variable controls the GPU-side constant buffer related to the above structure

