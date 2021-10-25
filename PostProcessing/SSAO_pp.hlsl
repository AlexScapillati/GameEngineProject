//--------------------------------------------------------------------------------------
// Screen Space Ambient Occlusion Post-Processing Pixel Shader
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

// Funtion that returns the normal given the depth value, the pixel uv and the texel size
float3 CalculateNormal(float depth, float2 texcoords, float2 texelSize)
{
    // Amount of the offsets based on the texel size
    const float2 offset1 = float2(0.0, texelSize.y);
    const float2 offset2 = float2(texelSize.x, 0.0);
 
    // Sample the depth texture in the different offsets
    float depth1 = DepthTexture.Sample(PointSample, texcoords + offset1).r;
    float depth2 = DepthTexture.Sample(PointSample, texcoords + offset2).r;
  
    // Calculate the position of the points relative to their offsets
    float3 p1 = float3(offset1, depth1 - depth);
    float3 p2 = float3(offset2, depth2 - depth);
  
    // Cross product between this two points to get the vector
    float3 normal = cross(p1, p2);
    
    // Flip the Z
    normal.z = -normal.z;
  
    // Get the normal
    return normalize(normal);
}


float4 main(PostProcessingInput In) : SV_TARGET
{
    // Number of sphere samples
    const int kNumSamples = 16;
    
    // Already defined vectors to sample the surroundings
    const float3 kSampleSphere[kNumSamples] =
    {
        float3(0.5381, 0.1856, -0.4319),    float3(0.1379, 0.2486, 0.4430),
        float3(0.3371, 0.5679, -0.0057),    float3(-0.6999, -0.0451, -0.0019),
        float3(0.0689, -0.1598, -0.8547),   float3(0.0560, 0.0069, -0.1843),
        float3(-0.0146, 0.1402, 0.0762),    float3(0.0100, -0.1924, -0.0344),
        float3(-0.3577, -0.5301, -0.4358),  float3(-0.3169, 0.1063, 0.0158),
        float3(0.0103, -0.5869, 0.0046),    float3(-0.0897, -0.4940, 0.3287),
        float3(0.7119, -0.0154, -0.0918),   float3(-0.0533, 0.0596, -0.5411),
        float3(0.0352, -0.0631, 0.5460),    float3(-0.4776, 0.2847, -0.0271)
    };
    
    // Get the size of the depth map (width, height)
    float2 depthMapSize;
    DepthTexture.GetDimensions(depthMapSize.x, depthMapSize.y);
  
    // Sample the depth map
    float depth = DepthTexture.Sample(PointSample, In.sceneUV).r;
    
    // Get the position of the texel
    // This is not the actual position of the pixel in world space
    float3 position = float3(In.sceneUV, depth);
    
    // Get the normal based on that pixel
    float3 normal = CalculateNormal(depth, In.sceneUV, 1.0 / depthMapSize);
    
    // Get the radius of the ray from CPU devided by the current depth
    float depthRadius = gSsaoRadius / depth;
    
    // Initialize the occlusion value
    float occlusion = 0.0;
    
    // For all the samples
    for (int i = 0; i < kNumSamples; i++)
    {
        // Project the ray in the correct direction
        float3 ray = depthRadius * kSampleSphere[i];
        float3 projectedRay = position + sign(dot(ray, normal)) * ray;
    
        // Sample the depth at that ray point
        float occluderDepth = DepthTexture.Sample(PointSample, projectedRay.xy).r;
        
        // Calculate the difference compared to the current depth
        float difference = depth - occluderDepth;
    
        // Increment the occlusion 
        // Depending on the falloff value (CPU side) (if the difference is greater the "step" function will return 1)
        // Interpolate the difference by the falloff and the area (cpu side)
        occlusion += step(gSsaoFalloff, difference) * (1.0 - smoothstep(gSsaoFalloff, gSsaoArea, difference));
    }
  
    float ao = 1.0 - gSsaoStrenght * occlusion * (1.0 / kNumSamples);
    return float4(ao.rrr, 1.0f);
}
