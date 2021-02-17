//--------------------------------------------------------------------------------------
// Commonly used definitions across entire project
//--------------------------------------------------------------------------------------
#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#include "CVector3.h"
#include "CMatrix4x4.h"

#include <d3d11.h>
#include <string>



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global Variables from various files available to other files. "extern" means
// this variable is defined in another file somewhere. We should use classes and avoid
// use of globals, but done this way to keep code simpler so the DirectX content is
// clearer. However, try to architect your own code in a better way.

// Windows variables
extern HWND gHWnd;

// Viewport size
extern int gViewportWidth;
extern int gViewportHeight;


// Important DirectX variables
extern ID3D11Device*           gD3DDevice;
extern ID3D11DeviceContext*    gD3DContext;

extern IDXGISwapChain*           gSwapChain;
extern ID3D11RenderTargetView*   gBackBufferRenderTarget; // Back buffer is where we render to
extern ID3D11DepthStencilView*   gDepthStencil;           // The depth buffer contains a depth for each back buffer pixel
extern ID3D11ShaderResourceView* gDepthShaderView;        // Allows access to the depth buffer as a texture for certain specialised shaders


// Input constsnts
extern const float ROTATION_SPEED;
extern const float MOVEMENT_SPEED;


// A global error message to help track down fatal errors - set it to a useful message
// when a serious error occurs
extern std::string gLastError;

extern std::string gMediaFolder;

//--------------------------------------------------------------------------------------
// Light Structures
//--------------------------------------------------------------------------------------

struct sLight
{
    CVector3 position;
	float padding;
    CVector3 colour;
    uint32_t  numLights;
};


struct sSpotLight
{
    CVector3 colour;
    float pad;
    CVector3 pos;
    uint32_t numLights;
    CVector3 facing;          //the direction facing of the light 
    float cosHalfAngle;       //pre calculate this in the c++ side, for performance reasons
    CMatrix4x4 viewMatrix;    //the light view matrix (as it was a camera)
    CMatrix4x4 projMatrix;    //--"--
};

struct sDirLights
{
    CVector3 colour;
    float pad;
    CVector3 facing;
    uint32_t numLights;
    CMatrix4x4 viewMatrix;    //the light view matrix (as it was a camera)
    CMatrix4x4 projMatrix;    //--"--
};

struct sPointLights
{
    CVector3 colour;
    float pad;
    CVector3 position;
    uint32_t numLights;
    CMatrix4x4 viewMatrices[6];    //the light view matrix (as it was a camera)
    CMatrix4x4 projMatrices[6];    //--"--
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

    CVector3   cameraPosition;
	float      frameTime;      // This app does updates on the GPU so we pass over the frame update time
};

extern PerFrameConstants gPerFrameConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerFrameConstantBuffer; // This variable controls the GPU-side constant buffer matching to the above structure

struct PerFrameLights
{
    sLight lights[MAX_LIGHTS];
};

extern PerFrameLights gPerFrameLightsConstants;
extern ID3D11Buffer* gPerFrameLightsConstBuffer;

struct PerFrameSpotLights
{
    sSpotLight spotLights[MAX_LIGHTS];
};

extern PerFrameSpotLights gPerFrameSpotLightsConstants;
extern ID3D11Buffer* gPerFrameSpotLightsConstBuffer;

struct PerFrameDirLights
{
    sDirLights dirLights[MAX_LIGHTS];
};

extern PerFrameDirLights gPerFrameDirLightsConstants;
extern ID3D11Buffer* gPerFrameDirLightsConstBuffer;

struct PerFramePointLights
{
    sPointLights pointLights[MAX_LIGHTS];
};

extern PerFramePointLights gPerFramePointLightsConstants;
extern ID3D11Buffer* gPerFramePointLightsConstBuffer;

static const int MAX_BONES = 64;

// This is the matrix that positions the next thing to be rendered in the scene. Unlike the structure above this data can be
// updated and sent to the GPU several times every frame (once per model). However, apart from that it works in the same way.
struct PerModelConstants
{
    CMatrix4x4 worldMatrix;

    CVector3   objectColour;  // Allows each light model to be tinted to match the light colour they cast
	float      parallaxDepth; // Used in the geometry shader to control how much the polygons are exploded outwards

	CMatrix4x4 boneMatrices[MAX_BONES];
};
extern PerModelConstants gPerModelConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerModelConstantBuffer; // This variable controls the GPU-side constant buffer related to the above structure


#endif //_COMMON_H_INCLUDED_
