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

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_TARGET
{
    
    float2 dimentions;
    
    // Get the texture dimentions (width, heigth)
    SceneTexture.GetDimensions(dimentions.x, dimentions.y);
 
    float Pi = 6.28318530718; // Pi*2
    
    // This is the "radius" of blurring
    float2 radius = gBlurSize / dimentions.xy;
    
    // Normalized pixel coordinates (from 0 to 1)
    float2 uv = input.sceneUV / dimentions.xy;
    
    // The scene colour
    float3 res = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    
    // For every direction (for every "ray" starting from the current pixel to the cos and sin of the direction)
    for (float currDir = 0.0f; currDir < Pi; currDir += Pi / gBlurDirections)
    {
        // For every point along the direction (given by 1 / gBlurQuality)
        for (float i = 1.0f / gBlurQuality; i <= 1.0f; i += 1.0f / gBlurQuality)
        {
            // calculate the offset
            float2 offset = float2(cos(currDir), sin(currDir)) * radius * i;
            
            // sample at the offset
            res += SceneTexture.Sample(PointSample, input.sceneUV + offset);
        }
    }
    
    // Divide by the quality and the directions 
    res /= gBlurQuality * gBlurDirections - 15.0f;
    
    return float4(res, 1.0f);
}