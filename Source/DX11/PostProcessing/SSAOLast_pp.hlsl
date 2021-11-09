//--------------------------------------------------------------------------------------
// SSAO Last Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------

#include "../Shaders/Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when

Texture2D SSAOTexture : register(t1); // The ssao texture that we just calculated

float4 main(PostProcessingInput input) : SV_TARGET
{
    
    return SceneTexture.Sample(PointSample, input.sceneUV) * SSAOTexture.Sample(PointSample, input.sceneUV);
    
}