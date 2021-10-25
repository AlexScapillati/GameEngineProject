//--------------------------------------------------------------------------------------
// Loading GPU shaders
// Creation of constant buffers to help send C++ values to shaders each frame
//--------------------------------------------------------------------------------------

#ifndef SHADER_CPP_INCLUDED_
#define SHADER_CPP_INCLUDED_

#include "Shader.h"
#include "Common.h"
#include <d3dcompiler.h>
#include <fstream>
#include <vector>

//*******************************
//**** Post-processing shader DirectX objects
// These are also added to Shader.h
ComPtr<ID3D11PixelShader>	gSsaoPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gCopyPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gTintPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gBurnPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gBloomPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gSpiralPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gGodRaysPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gDistortPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gSsaoLastPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gHeatHazePostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gBloomLastPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gGreyNoisePostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gGaussionBlurPostProcess = nullptr;
ComPtr<ID3D11PixelShader>	gChromaticAberrationPostProcess = nullptr;

ComPtr<ID3D11VertexShader>  g2DPolygonVertexShader = nullptr;
ComPtr<ID3D11VertexShader>  g2DQuadVertexShader = nullptr;


// Load a vertex shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure.
ID3D11VertexShader* LoadVertexShader(const std::string& shaderName)
{
	// Open compiled shader object file
	std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
	if (!shaderFile.is_open())
	{
		return nullptr;
	}

	// Read file into vector of chars
	const std::streamoff fileSize = shaderFile.tellg();
	shaderFile.seekg(0, std::ios::beg);
	std::vector<char> byteCode(fileSize);
	shaderFile.read(&byteCode[0], fileSize);
	if (shaderFile.fail())
	{
		return nullptr;
	}

	// Create shader object from loaded file (we will use the object later when rendering)
	ID3D11VertexShader* shader;
	const auto hr = gD3DDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, &shader);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return shader;
}

// Load a geometry shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure.
// Basically the same code as above but for pixel shaders
ID3D11GeometryShader* LoadGeometryShader(const std::string& shaderName)
{
	// Open compiled shader object file
	std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
	if (!shaderFile.is_open())
	{
		return nullptr;
	}

	// Read file into vector of chars
	const std::streamoff fileSize = shaderFile.tellg();
	shaderFile.seekg(0, std::ios::beg);
	std::vector<char>byteCode(fileSize);
	shaderFile.read(&byteCode[0], fileSize);
	if (shaderFile.fail())
	{
		return nullptr;
	}

	// Create shader object from loaded file (we will use the object later when rendering)
	ID3D11GeometryShader* shader;
	const auto hr = gD3DDevice->CreateGeometryShader(byteCode.data(), byteCode.size(), nullptr, &shader);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return shader;
}

// Special method to load a geometry shader that can use the stream-out stage, Use like the other functions in this file except
// also pass the stream out declaration, number of entries in the declaration and the size of each output element.
// The returned pointer needs to be released before quitting. Returns nullptr on failure.
ID3D11GeometryShader* LoadStreamOutGeometryShader(const std::string& shaderName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries, unsigned int soStride)
{
	// Open compiled shader object file
	std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
	if (!shaderFile.is_open())
	{
		return nullptr;
	}

	// Read file into vector of chars
	const std::streamoff fileSize = shaderFile.tellg();
	shaderFile.seekg(0, std::ios::beg);
	std::vector<char>byteCode(fileSize);
	shaderFile.read(&byteCode[0], fileSize);
	if (shaderFile.fail())
	{
		return nullptr;
	}

	// Create shader object from loaded file (we will use the object later when rendering)
	ID3D11GeometryShader* shader;
	const auto hr = gD3DDevice->CreateGeometryShaderWithStreamOutput(byteCode.data(), byteCode.size(),
		soDecl, soNumEntries, &soStride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &shader);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return shader;
}

// Load a pixel shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure.
// Basically the same code as above but for pixel shaders
ID3D11PixelShader* LoadPixelShader(const std::string& shaderName)
{
	// Open compiled shader object file
	std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
	if (!shaderFile.is_open())
	{
		return nullptr;
	}

	// Read file into vector of chars
	const std::streamoff fileSize = shaderFile.tellg();
	shaderFile.seekg(0, std::ios::beg);
	std::vector<char>byteCode(fileSize);
	shaderFile.read(&byteCode[0], fileSize);
	if (shaderFile.fail())
	{
		return nullptr;
	}

	// Create shader object from loaded file (we will use the object later when rendering)
	ID3D11PixelShader* shader;
	const auto hr = gD3DDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, &shader);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return shader;
}

// Very advanced topic: When creating a vertex layout for geometry (see Scene.cpp), you need the signature
// (bytecode) of a shader that uses that vertex layout. This is an annoying requirement and tends to create
// unnecessary coupling between shaders and vertex buffers.
// This is a trick to simplify things - pass a vertex layout to this function and it will write and compile
// a temporary shader to match. You don't need to know about the actual shaders in use in the app.
// Release the signature (called a ID3DBlob!) after use. Returns nullptr on failure.
ID3DBlob* CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements)
{
	std::string shaderSource = "float4 main(";
	for (auto elt = 0; elt < numElements; ++elt)
	{
		auto& format = vertexLayout[elt].Format;
		// This list should be more complete for production use
		if (format == DXGI_FORMAT_R32G32B32A32_FLOAT) shaderSource += "float4";
		else if (format == DXGI_FORMAT_R32G32B32_FLOAT)    shaderSource += "float3";
		else if (format == DXGI_FORMAT_R32G32_FLOAT)       shaderSource += "float2";
		else if (format == DXGI_FORMAT_R32_FLOAT)          shaderSource += "float";
		else if (format == DXGI_FORMAT_R8G8B8A8_UINT)      shaderSource += "uint4";
		else return nullptr; // Unsupported type in layout

		const auto index = static_cast<uint8_t>(vertexLayout[elt].SemanticIndex);
		std::string semanticName = vertexLayout[elt].SemanticName;
		semanticName += ('0' + index);

		shaderSource += " ";
		shaderSource += semanticName;
		shaderSource += " : ";
		shaderSource += semanticName;
		if (elt != numElements - 1)  shaderSource += " , ";
	}
	shaderSource += ") : SV_Position {return 0;}";

	ID3DBlob* compiledShader;
	const auto hr = D3DCompile(shaderSource.c_str(), shaderSource.length(), NULL, NULL, NULL, "main",
		"vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, &compiledShader, NULL);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return compiledShader;
}

//--------------------------------------------------------------------------------------
// Constant buffer creation / destruction
//--------------------------------------------------------------------------------------

// Constant Buffers are a way of passing data from C++ to the GPU. They are called constants but that only means
// they are constant for the duration of a single GPU draw call. The "constants" correspond to variables in C++
// that we will change per-model, or per-frame etc.
//
// We typically set up a C++ structure to exactly match the values we need in a shader and then create a constant
// buffer the same size as the structure. That makes updating values from C++ to shader easy - see the main code.

void LoadDefaultShaders()
{
	//-------------------------------------
	// Post Processing Shaders
	//-------------------------------------

	gCopyPostProcess.Attach(LoadPixelShader("PostProcessing/Copy_pp"));
	gTintPostProcess.Attach(LoadPixelShader("PostProcessing/Tint_pp"));
	gBurnPostProcess.Attach(LoadPixelShader("PostProcessing/Burn_pp"));
	gSsaoPostProcess.Attach(LoadPixelShader("PostProcessing/SSAO_pp"));
	gBloomPostProcess.Attach(LoadPixelShader("PostProcessing/Bloom_pp"));
	gSpiralPostProcess.Attach(LoadPixelShader("PostProcessing/Spiral_pp"));
	gGodRaysPostProcess.Attach(LoadPixelShader("PostProcessing/GodRays_pp"));
	gDistortPostProcess.Attach(LoadPixelShader("PostProcessing/Distort_pp"));
	g2DQuadVertexShader.Attach(LoadVertexShader("PostProcessing/2DQuad_pp"));
	gSsaoLastPostProcess.Attach(LoadPixelShader("PostProcessing/SSAOLast_pp"));
	gHeatHazePostProcess.Attach(LoadPixelShader("PostProcessing/HeatHaze_pp"));
	gBloomLastPostProcess.Attach(LoadPixelShader("PostProcessing/BloomLast_pp"));
	gGreyNoisePostProcess.Attach(LoadPixelShader("PostProcessing/GreyNoise_pp"));
	g2DPolygonVertexShader.Attach(LoadVertexShader("PostProcessing/2DPolygon_pp"));
	gGaussionBlurPostProcess.Attach(LoadPixelShader("PostProcessing/GaussionBlur_pp"));
	gChromaticAberrationPostProcess.Attach(LoadPixelShader("PostProcessing/ChromaticAberration_pp"));

	//-------------------------------------
	// Default Shaders
	//-------------------------------------

	gDepthOnlyPixelShader.Attach(LoadPixelShader("Shaders/DepthOnly_ps"));
	gDepthOnlyNormalPixelShader.Attach(LoadPixelShader("Shaders/DepthOnlyNormal_ps"));
	gBasicTransformVertexShader.Attach(LoadVertexShader("Shaders/BasicTransform_vs"));

	gPBRVertexShader.Attach(LoadVertexShader("Shaders/PBRNoNormals_vs"));
	gPBRNormalVertexShader.Attach(LoadVertexShader("Shaders/PBR_vs"));

	gPBRPixelShader.Attach(LoadPixelShader("Shaders/PBRNoNormals_ps"));
	gPBRNormalPixelShader.Attach(LoadPixelShader("Shaders/PBR_ps"));

	gTintedTexturePixelShader.Attach(LoadPixelShader("Shaders/TintedTexture_ps"));
	gSkyPixelShader.Attach(LoadPixelShader("Shaders/Sky_ps"));
	gSkyVertexShader.Attach(LoadVertexShader("Shaders/Sky_vs"));

	// Check for all the shaders if they are being loaded
	if (gDepthOnlyPixelShader == nullptr || gBasicTransformVertexShader == nullptr ||
		g2DQuadVertexShader == nullptr || gCopyPostProcess == nullptr ||
		gTintPostProcess == nullptr || gHeatHazePostProcess == nullptr ||
		gGreyNoisePostProcess == nullptr || gBurnPostProcess == nullptr ||
		gDistortPostProcess == nullptr || gSpiralPostProcess == nullptr ||
		g2DPolygonVertexShader == nullptr || gChromaticAberrationPostProcess == nullptr ||
		gGaussionBlurPostProcess == nullptr || gSsaoPostProcess == nullptr ||
		gBloomPostProcess == nullptr || gBloomLastPostProcess == nullptr ||
		gSsaoLastPostProcess == nullptr || gGodRaysPostProcess == nullptr ||
		gPBRVertexShader == nullptr || gPBRNormalVertexShader == nullptr ||
		gPBRPixelShader == nullptr || gPBRNormalPixelShader == nullptr ||
		gTintedTexturePixelShader == nullptr || gSkyPixelShader == nullptr ||
		gSkyVertexShader == nullptr)
	{
		throw std::runtime_error("Error loading default shaders");
	}
}

void ReleaseDefaultShaders()
{
	if (gDepthOnlyPixelShader)			  gDepthOnlyPixelShader = nullptr;
	if (gBasicTransformVertexShader)	  gBasicTransformVertexShader = nullptr;
	if (gHeatHazePostProcess)			  gHeatHazePostProcess = nullptr;
	if (gSpiralPostProcess)				  gSpiralPostProcess = nullptr;
	if (gDistortPostProcess)			  gDistortPostProcess = nullptr;
	if (gBurnPostProcess)				  gBurnPostProcess = nullptr;
	if (gGreyNoisePostProcess)			  gGreyNoisePostProcess = nullptr;
	if (gTintPostProcess)				  gTintPostProcess = nullptr;
	if (gCopyPostProcess)				  gCopyPostProcess = nullptr;
	if (g2DPolygonVertexShader)			  g2DPolygonVertexShader = nullptr;
	if (g2DQuadVertexShader)			  g2DQuadVertexShader = nullptr;
	if (gChromaticAberrationPostProcess)  gChromaticAberrationPostProcess = nullptr;
	if (gGaussionBlurPostProcess)		  gGaussionBlurPostProcess = nullptr;
	if (gBloomPostProcess)				  gBloomPostProcess = nullptr;
	if (gBloomLastPostProcess)			  gBloomLastPostProcess = nullptr;
	if (gSsaoLastPostProcess)			  gSsaoLastPostProcess = nullptr;
	if (gSsaoPostProcess)				  gSsaoPostProcess = nullptr;
	if (gGodRaysPostProcess)			  gGodRaysPostProcess = nullptr;
	if (gDepthOnlyNormalPixelShader)	  gDepthOnlyNormalPixelShader = nullptr;

	if (gPBRVertexShader)				  gPBRVertexShader = nullptr;
	if (gPBRNormalVertexShader)			  gPBRNormalVertexShader = nullptr;
	if (gPBRPixelShader)				  gPBRPixelShader = nullptr;
	if (gPBRNormalPixelShader)			  gPBRNormalPixelShader = nullptr;

	if (gTintedTexturePixelShader)		  gTintedTexturePixelShader = nullptr;
	if (gSkyPixelShader)				  gSkyPixelShader = nullptr;
	if (gSkyVertexShader)				  gSkyVertexShader = nullptr;

}

// Create and return a constant buffer of the given size
// The returned pointer needs to be released before quitting. Returns nullptr on failure.
ID3D11Buffer* CreateConstantBuffer(int size)
{
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = 16 * ((size + 15) / 16);     // Constant buffer size must be a multiple of 16 - this maths rounds up to the nearest multiple
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;             // Indicates that the buffer is frequently updated
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU is only going to write to the constants (not read them)
	cbDesc.MiscFlags = 0;
	ID3D11Buffer* constantBuffer;
	const auto hr = gD3DDevice->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return constantBuffer;
}

#endif