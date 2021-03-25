//--------------------------------------------------------------------------------------
// Depth-Only Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader that only outputs the depth of a pixel - used to render the scene from the point
// of view of the lights in preparation for shadow mapping

#include "Common.hlsli" // Shaders can also use include files - note the extension

Texture2D OpacityMap : register(t0);
SamplerState texSampler : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Shader used when rendering the shadow map depths. The pixel shader doesn't really need to calculate a pixel colour
// since we are only writing to the depth buffer.

void main(LightingPixelShaderInput i)
{
    if (gHasOpacityMap == 1.0f)
    {
        if (OpacityMap.Sample(texSampler, i.uv).r == 0.0f)
            discard;
    }
}

// If the depth buffer is enabled, this will output depth but no colour (C++ side render target should be nullptr)
