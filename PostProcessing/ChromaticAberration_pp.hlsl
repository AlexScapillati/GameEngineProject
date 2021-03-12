//--------------------------------------------------------------------------------------
// Chromatic Aberration Post-Processing Pixel Shader
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
    
    float2 centre = (0.5f, 0.5f);
    
    float distFromCentre = input.sceneUV - centre;
    
    float amount = gCAAmount;
    
    float3 col;
    col.r = SceneTexture.Sample(PointSample, float2(input.sceneUV.x + amount * distFromCentre, input.sceneUV.y)).r;
    col.g = SceneTexture.Sample(PointSample, input.sceneUV).g;
    col.b = SceneTexture.Sample(PointSample, float2(input.sceneUV.x - amount * distFromCentre, input.sceneUV.y)).b;

    col *= (1.0 - amount * 0.5);
    
	
    return float4(col, 1.0f);
}