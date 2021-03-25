//--------------------------------------------------------------------------------------
// Texture Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader simply samples a diffuse texture map and tints with colours from vertex shader

#include "Common.hlsli" // Shaders can also use include files - note the extension

Texture2D SpecularDiffuseTexture : register(t0);

SamplerState TexSampler : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(LightingPixelShaderInput input) : SV_TARGET
{
    return SpecularDiffuseTexture.Sample(TexSampler,input.uv);
}