//--------------------------------------------------------------------------------------
// Loading GPU shaders
// Creation of constant buffers to help send C++ values to shaders each frame
//--------------------------------------------------------------------------------------

#ifndef SHADER_CPP_INCLUDED_
#define SHADER_CPP_INCLUDED_

#include "DX11Engine.h"
#include <d3dcompiler.h>
#include <fstream>
#include <vector>

// Load a vertex shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure.
ID3D11VertexShader* CDX11Engine::LoadVertexShader(const std::string& shaderName)
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
	const auto          hr = GetDevice()->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, &shader);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return shader;
}

// Load a geometry shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure.
// Basically the same code as above but for pixel shaders
ID3D11GeometryShader* CDX11Engine::LoadGeometryShader(const std::string& shaderName)
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
	ID3D11GeometryShader* shader;
	const auto            hr = GetDevice()->CreateGeometryShader(byteCode.data(), byteCode.size(), nullptr, &shader);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return shader;
}

// Special method to load a geometry shader that can use the stream-out stage, Use like the other functions in this file except
// also pass the stream out declaration, number of entries in the declaration and the size of each output element.
// The returned pointer needs to be released before quitting. Returns nullptr on failure.
ID3D11GeometryShader* CDX11Engine::LoadStreamOutGeometryShader(const std::string&          shaderName,
                                                               D3D11_SO_DECLARATION_ENTRY* soDecl,
                                                               unsigned int                soNumEntries,
                                                               unsigned int                soStride)
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
	ID3D11GeometryShader* shader;
	const auto            hr = GetDevice()->CreateGeometryShaderWithStreamOutput(byteCode.data(),
		byteCode.size(),
		soDecl,
		soNumEntries,
		&soStride,
		1,
		D3D11_SO_NO_RASTERIZED_STREAM,
		nullptr,
		&shader);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return shader;
}

// Load a pixel shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure.
// Basically the same code as above but for pixel shaders
ID3D11PixelShader* CDX11Engine::LoadPixelShader(const std::string& shaderName)
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
	ID3D11PixelShader* shader;
	const auto         hr = GetDevice()->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, &shader);
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
ID3DBlob* CDX11Engine::CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements)
{
	std::string shaderSource = "float4 main(";
	for (auto elt = 0; elt < numElements; ++elt)
	{
		auto& format = vertexLayout[elt].Format;
		// This list should be more complete for production use
		if (format == DXGI_FORMAT_R32G32B32A32_FLOAT) shaderSource += "float4";
		else if (format == DXGI_FORMAT_R32G32B32_FLOAT) shaderSource += "float3";
		else if (format == DXGI_FORMAT_R32G32_FLOAT) shaderSource += "float2";
		else if (format == DXGI_FORMAT_R32_FLOAT) shaderSource += "float";
		else if (format == DXGI_FORMAT_R8G8B8A8_UINT) shaderSource += "uint4";
		else return nullptr; // Unsupported type in layout

		const auto  index        = static_cast<uint8_t>(vertexLayout[elt].SemanticIndex);
		std::string semanticName = vertexLayout[elt].SemanticName;
		semanticName += ('0' + index);

		shaderSource += " ";
		shaderSource += semanticName;
		shaderSource += " : ";
		shaderSource += semanticName;
		if (elt != numElements - 1) shaderSource += " , ";
	}
	shaderSource += ") : SV_Position {return 0;}";

	ID3DBlob*  compiledShader;
	const auto hr = D3DCompile(shaderSource.c_str(),
	                           shaderSource.length(),
	                           NULL,
	                           NULL,
	                           NULL,
	                           "main",
	                           "vs_5_0",
	                           D3DCOMPILE_OPTIMIZATION_LEVEL0,
	                           0,
	                           &compiledShader,
	                           NULL);
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

void CDX11Engine::LoadDefaultShaders()
{
	//-------------------------------------
	// Post Processing Shaders
	//-------------------------------------

	mCopyPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/Copy_pp"));
	mTintPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/Tint_pp"));
	mBurnPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/Burn_pp"));
	mSsaoPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/SSAO_pp"));
	mBloomPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/Bloom_pp"));
	mSpiralPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/Spiral_pp"));
	mGodRaysPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/GodRays_pp"));
	mDistortPostProcess.Attach(				LoadPixelShader(	"Source/DX11/PostProcessing/Distort_pp"));
	m2DQuadVertexShader.Attach(				LoadVertexShader(	"Source/DX11/PostProcessing/2DQuad_pp"));
	mSsaoLastPostProcess.Attach(			LoadPixelShader(	"Source/DX11/PostProcessing/SSAOLast_pp"));
	mHeatHazePostProcess.Attach(			LoadPixelShader(	"Source/DX11/PostProcessing/HeatHaze_pp"));
	mBloomLastPostProcess.Attach(			LoadPixelShader(	"Source/DX11/PostProcessing/BloomLast_pp"));
	mGreyNoisePostProcess.Attach(			LoadPixelShader(	"Source/DX11/PostProcessing/GreyNoise_pp"));
	m2DPolygonVertexShader.Attach(			LoadVertexShader(	"Source/DX11/PostProcessing/2DPolygon_pp"));
	mGaussionBlurPostProcess.Attach(		LoadPixelShader(	"Source/DX11/PostProcessing/GaussionBlur_pp"));
	mChromaticAberrationPostProcess.Attach(	LoadPixelShader(	"Source/DX11/PostProcessing/ChromaticAberration_pp"));

	//-------------------------------------
	// Default Shaders
	//-------------------------------------

	mDepthOnlyPixelShader.Attach(LoadPixelShader("Source/DX11/Shaders/DepthOnly_ps"));
	mDepthOnlyNormalPixelShader.Attach(LoadPixelShader("Source/DX11/Shaders/DepthOnlyNormal_ps"));
	mBasicTransformVertexShader.Attach(LoadVertexShader("Source/DX11/Shaders/BasicTransform_vs"));
	mPbrVertexShader.Attach(LoadVertexShader("Source/DX11/Shaders/PBRNoNormals_vs"));
	mPbrNormalVertexShader.Attach(LoadVertexShader("Source/DX11/Shaders/PBR_vs"));
	mPbrPixelShader.Attach(LoadPixelShader("Source/DX11/Shaders/PBRNoNormals_ps"));
	mPbrNormalPixelShader.Attach(LoadPixelShader("Source/DX11/Shaders/PBR_ps"));
	mTintedTexturePixelShader.Attach(LoadPixelShader("Source/DX11/Shaders/TintedTexture_ps"));
	mSkyPixelShader.Attach(LoadPixelShader("Source/DX11/Shaders/Sky_ps"));
	mSkyVertexShader.Attach(LoadVertexShader("Source/DX11/Shaders/Sky_vs"));

	// Check for all the shaders if they are being loaded
	if (mDepthOnlyPixelShader == nullptr || mBasicTransformVertexShader == nullptr ||
		m2DQuadVertexShader == nullptr || mCopyPostProcess == nullptr ||
		mTintPostProcess == nullptr || mHeatHazePostProcess == nullptr ||
		mGreyNoisePostProcess == nullptr || mBurnPostProcess == nullptr ||
		mDistortPostProcess == nullptr || mSpiralPostProcess == nullptr ||
		m2DPolygonVertexShader == nullptr || mChromaticAberrationPostProcess == nullptr ||
		mGaussionBlurPostProcess == nullptr || mSsaoPostProcess == nullptr ||
		mBloomPostProcess == nullptr || mBloomLastPostProcess == nullptr ||
		mSsaoLastPostProcess == nullptr || mGodRaysPostProcess == nullptr ||
		mPbrVertexShader == nullptr || mPbrNormalVertexShader == nullptr ||
		mPbrPixelShader == nullptr || mPbrNormalPixelShader == nullptr ||
		mTintedTexturePixelShader == nullptr || mSkyPixelShader == nullptr ||
		mSkyVertexShader == nullptr) { throw std::runtime_error("Error loading default shaders"); }
}


// Create and return a constant buffer of the given size
// The returned pointer needs to be released before quitting. Returns nullptr on failure.
ID3D11Buffer* CDX11Engine::CreateConstantBuffer(int size)
{
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = 16 * ((size + 15) / 16);
	// Constant buffer size must be a multiple of 16 - this maths rounds up to the nearest multiple
	cbDesc.Usage          = D3D11_USAGE_DYNAMIC;    // Indicates that the buffer is frequently updated
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU is only going to write to the constants (not read them)
	cbDesc.MiscFlags      = 0;
	ID3D11Buffer* constantBuffer;
	const auto    hr = GetDevice()->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
	if (FAILED(hr))
	{
		return nullptr;
	}

	return constantBuffer;
}

#endif
