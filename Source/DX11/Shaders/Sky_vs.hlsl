
#include "Common.hlsli" // Shaders can also use include files - note the extension

LightingPixelShaderInput main(BasicVertex modelVertex)
{
    LightingPixelShaderInput output; // This is the data the pixel shader requires from this vertex shader
    
    output.projectedPosition = mul(float4(modelVertex.position, 1.0f), gViewProjectionMatrix);
    
    output.projectedPosition.z = output.projectedPosition.w;
    
    output.worldPosition = modelVertex.position;

    return output; // Ouput data sent down the pipeline (to the pixel shader)
}