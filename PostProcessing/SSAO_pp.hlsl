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

int kernelSize = 10;


float rand(float2 co)
{
    return 0.5 + (frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453)) * 0.5;
}
    

float3 normal_from_depth(float depth, float2 texcoords)
{
  
    const float2 offset1 = float2(0.0, 0.001);
    const float2 offset2 = float2(0.001, 0.0);
 
    float depth1 = DepthTexture.Sample(PointSample, texcoords + offset1).r;
    float depth2 = DepthTexture.Sample(PointSample, texcoords + offset2).r;
  
    float3 p1 = float3(offset1, depth1 - depth);
    float3 p2 = float3(offset2, depth2 - depth);
  
    float3 normal = cross(p1, p2);
    normal.z = -normal.z;
  
    return normalize(normal);
}

float4 main(PostProcessingInput input) : SV_TARGET
{
    
    float depth = DepthTexture.Sample(PointSample, input.sceneUV).r;
    
    depth = depth - 0.99f;
    depth = depth * 100.0f;
    
   
    return float4(depth.rrr, 1.0);
    
}
