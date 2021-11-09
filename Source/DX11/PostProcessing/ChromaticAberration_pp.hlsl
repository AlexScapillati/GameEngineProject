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
    // Get the centre of the screen in UV space
    // Since the uv range is -1, 1 the distance from the centre is the length of the actual scene UV
    float2 vFromCentre = input.sceneUV - float2(0.5f,0.5f);
    
    float dist = length(vFromCentre);
    
    float vignette = 1.0 - smoothstep(gCAEdge, gCAEdge - gCAFalloff, dist);
    
    float offset = vFromCentre * vignette * gCAAmount;
    
    // Sample the scene texture and shift by a dynamic amount the sampling offset
    // The amount is directional proportionate to the distant from the centre of the texture
    // Leave the green channel
    float3 col;
    col.r = SceneTexture.Sample(PointSample, float2(input.sceneUV) + offset).r;
    col.g = SceneTexture.Sample(PointSample, input.sceneUV).g;
    col.b = SceneTexture.Sample(PointSample, float2(input.sceneUV) - offset).b;

    // Scale it from 0 to 1
    col *= (1.0 - gCAAmount * 0.5);
    
	// Return 
    return float4(col, 1.0f);
}