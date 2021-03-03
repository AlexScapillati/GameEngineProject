//--------------------------------------------------------------------------------------
// Gaussion Blur Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------

#include "../Shaders/Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when

//ambient occlusion needs a depth texture
Texture2D DepthTexture : register(t1);

float4 main(PostProcessingInput input) : SV_TARGET
{
    
    float4 col = DepthTexture.Sample(PointSample, input.sceneUV);
    
    col = col - 0.99f;
    col = col * 100.0f; 
    
    return float4(col.rrr,1.0f);
}
