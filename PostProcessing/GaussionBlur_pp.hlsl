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
                                          // post-processing so this sampler will use "point sampling" - no filtering


const float offset[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
const float weight[] =
{
    0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162
};
 

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_TARGET
{
    
    float3 res;
    
    SceneTexture.GetDimensions(0, res.x, res.y, res.z);
 
    float Pi = 6.28318530718; // Pi*2
    
   
    float2 Radius = gBlurSize / res.xy;
    
    // Normalized pixel coordinates (from 0 to 1)
    float2 uv = input.sceneUV / res.xy;
    
    // Pixel colour
    float3 Color = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    
    // Blur calculations
    for (float d = 0.0; d < Pi; d += Pi / gBlurDirections)
    {
        for (float i = 1.0 / gBlurQuality; i <= 1.0; i += 1.0 / gBlurQuality)
        {
            Color += SceneTexture.Sample(PointSample, input.sceneUV + float2(cos(d), sin(d)) * Radius * i);
        }
    }
    
    // Output to screen
    Color /= gBlurQuality * gBlurDirections - 15.0;
    
    return float4(Color, 1.0f);
}