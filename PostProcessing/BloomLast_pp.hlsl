//--------------------------------------------------------------------------------------
// Bloom Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------

#include "../Shaders/Common.hlsli" // Shaders can also use include files - note the extensionion

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when

Texture2D BloomTexture : register(t1); //this will be the blurried texture with only the most bright zones

float4 main(PostProcessingInput input) : SV_TARGET
{
    
    return SceneTexture.Sample(PointSample, input.sceneUV) + BloomTexture.Sample(PointSample, input.sceneUV);
    
}