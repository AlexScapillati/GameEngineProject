//--------------------------------------------------------------------------------------
// Texture Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader simply samples a diffuse texture map and tints with colours from vertex shader

#include "Common.hlsli" // Shaders can also use include files - note the extension

TextureCube AmbientMap : register(t0);

SamplerState TexSampler : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(LightingPixelShaderInput input) : SV_TARGET
{
    return AmbientMap.Sample(TexSampler, input.projectedPosition.xyz);
}