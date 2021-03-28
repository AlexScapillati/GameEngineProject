
//--------------------------------------------------------------------------------------
// Screen Space Volumetric Light Scattering Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Just samples a pixel from the scene texture and multiplies it by a fixed colour to tint the scene

#include "../Shaders/Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); 

Texture2D DepthTexture : register(t1);

// Source: https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-13-volumetric-light-scattering-post-process

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_TARGET
{
    
    // Calculate vector from pixel to light source in screen space.    
    half2 deltaTexCoord = (input.sceneUV - gLightScreenPos.xy);
    
    // Divide by number of samples and scale by control factor.   
    deltaTexCoord *= 1.0f / gNumSamples * gDensity;
    
    // Store initial sample.    
    half3 color = SceneTexture.Sample(PointSample, input.sceneUV).rgb * 0.3f;
    
    float4 sceneCol = SceneTexture.Sample(PointSample, input.sceneUV);
    
    float2 dx = ddx(input.sceneUV);
    float2 dy = ddy(input.sceneUV);
    
    float depth = DepthTexture.SampleGrad(PointSample, input.sceneUV,dx,dy).r;
    
    //if (gLightScreenPos.x > 1.0 || gLightScreenPos.y > 1.0)
    //    return sceneCol;
    
    //if (gLightScreenPos.x < -1.0 || gLightScreenPos.y < -1.0)
    //    return sceneCol;
    
    if(depth > 0.99999f)
        return sceneCol;
    
    // Store variable texture coordinates 
    float2 texCoord = input.sceneUV;
    // Set up illumination decay factor.    
    half illuminationDecay = 1.0f;
    
    // Evaluate summation from Equation 3 gNUMSAMPLES iterations.   
    for (int i = 0; i < gNumSamples; i++)
    {
        // Step sample location along ray.     
        texCoord -= deltaTexCoord;
        // Retrieve sample at new location.    
        half3 sample = SceneTexture.SampleGrad(PointSample, texCoord,dx,dy) * 0.4;
        // Apply sample attenuation scale/decay factors.     
        sample *= illuminationDecay * gWeight;
        // Accumulate combined color.     
        color += sample;
        // Update exponential decay factor.     
        illuminationDecay *= gDecay;
    }
    
    color /= gNumSamples;
    
    // Output final color with a further scale control factor.    
    return float4(color * gExposure, 1) + sceneCol * 1.1;
}