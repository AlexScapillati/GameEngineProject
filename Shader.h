//--------------------------------------------------------------------------------------
// Loading GPU shaders
// Creation of constant buffers to help send C++ values to shaders each frame
//--------------------------------------------------------------------------------------
#ifndef _SHADER_H_INCLUDED_
#define _SHADER_H_INCLUDED_

#include <d3d11.h>
#include <string>

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global variables available to other files. "extern" means this variable is defined in another
// file somewhere. We should use classes and avoid use of globals, but done this way to keep code simpler
// so the DirectX content is clearer. However, try to architect your own code in a better way.

inline ID3D11PixelShader*  gDepthOnlyPixelShader = nullptr;
inline ID3D11VertexShader* gBasicTransformVertexShader = nullptr;
inline ID3D11PixelShader*  gDepthOnlyNormalPixelShader = nullptr;

inline ID3D11PixelShader*  gPBRPixelShader = nullptr;
inline ID3D11PixelShader*  gPBRNormalPixelShader = nullptr;
inline ID3D11VertexShader* gPBRVertexShader = nullptr;
inline ID3D11VertexShader* gPBRNormalVertexShader = nullptr;

inline ID3D11PixelShader* gTintedTexturePixelShader = nullptr;
inline ID3D11PixelShader* gSkyPixelShader = nullptr;


void LoadDefaultShaders();

void ReleaseDefaultShaders();

//--------------------------------------------------------------------------------------
// Constant buffer creation / destruction
//--------------------------------------------------------------------------------------

// Create and return a constant buffer of the given size
// The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11Buffer* CreateConstantBuffer(int size);

//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------

// Load a shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11VertexShader* LoadVertexShader(const std::string& shaderName);
ID3D11GeometryShader* LoadGeometryShader(const std::string& shaderName);
ID3D11PixelShader* LoadPixelShader(const std::string& shaderName);

// Special method to load a geometry shader that can use the stream-out stage, Use like the other functions in this file except
// also pass the stream out declaration, number of entries in the declaration and the size of each output element.
// The returned pointer needs to be released before quitting. Returns nullptr on failure.
ID3D11GeometryShader* LoadStreamOutGeometryShader(const std::string& shaderName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries, unsigned int soStride);

// Helper function. Returns nullptr on failure.
ID3DBlob* CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements);

#endif //_SHADER_H_INCLUDED_
