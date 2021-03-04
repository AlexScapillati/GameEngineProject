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

float luminance(float3 col)
{
    //relative luminance formula 
    // source : https://en.wikipedia.org/wiki/Relative_luminance
    return
    0.2126 * col.r +
    0.7152 * col.g +
    0.0722 * col.b;
}

float4 main(PostProcessingInput input) : SV_TARGET
{
    float3 col = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    
    if (luminance(col) > gBloomThreshold)
    {
        return float4(col, 1.0f);
    }
    else
        return 0;
}