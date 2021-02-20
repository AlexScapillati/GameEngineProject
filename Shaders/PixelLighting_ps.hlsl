//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader receives position and normal from the vertex shader and uses them to calculate
// lighting per pixel. Also samples a samples a diffuse + specular texture map and combines with light colour.

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D DiffuseSpecularMap : register(t0); // Textures here can contain a diffuse map (main colour) in their rgb channels and a specular map (shininess) in the a channel
SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic - this is the sampler used for the texture above
TextureCube ambientMap : register(t5);

Texture2DArray ShadowMaps : register(t6);
SamplerState PointClamp : register(s1);

static const int pcfCount = 8;
static const float totalTexels = (pcfCount * 2.0f + 1.0f) * (pcfCount * 2.0f + 1.0f);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(LightingPixelShaderInput input) : SV_Target
{
    // Normal might have been scaled by model scaling or interpolation so renormalise
    input.worldNormal = normalize(input.worldNormal);

	///////////////////////
	// Calculate lighting
    
    // Direction from pixel to camera
    const float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

	//// Lights ////
    
    float3 resDiffuse = gAmbientColour;
    float3 resSpecular = 0.0f;
    
    for (int i = 0; i < gLights[0].numLights && gLights[i].enabled; ++i)
    {
        const float3 lightDir = normalize(gLights[i].position - input.worldPosition);
        const float lightDist = length(input.worldPosition - gLights[i].position);

        const float3 diffuse = gLights[i].colour * max(dot(input.worldNormal, lightDir), 0) / lightDist;
        const float3 halfWay = normalize(lightDir + cameraDirection);
        const float3 specular = diffuse * pow(max(dot(input.worldNormal, halfWay), 0), gSpecularPower);
        
        resDiffuse += diffuse;
        resSpecular += specular;
    }
	
	//calculate lighting from directional lights

    const float depthAdjust = 0.0005f;
    
	//for each spot light
    for (int j = 0; j < gSpotLights[0].numLights && gSpotLights[j].enabled; ++j)
    {
        const float3 lightDir = normalize(gSpotLights[j].pos - input.worldPosition);

    	//if the pixel is in the light cone
        if (dot(lightDir, -gSpotLights[j].facing) > gSpotLights[j].cosHalfAngle)
        {
    		// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
			// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
			// These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
            const float4 viewPosition = mul(gSpotLights[j].viewMatrix, float4(input.worldPosition, 1.0f));
            const float4 projection = mul(gSpotLights[j].projMatrix, viewPosition);

			// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
			// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
            float2 shadowMapUV = 0.5f * projection.xy / projection.w + float2(0.5f, 0.5f);
            shadowMapUV.y = 1.0f - shadowMapUV.y; // Check if pixel is within light cone

			// Get depth of this pixel if it were visible from the light (another advanced projection step)
            const float depthFromLight = projection.z / projection.w - depthAdjust; //*** Adjustment so polygons don't shadow themselves
		
		
            //const float texelSize = 1.0f / 1024;
            //float total = 0.0f;
	
            //for (int x = -pcfCount; x <= pcfCount; x++)
            //{
            //    for (int y = -pcfCount; y <= pcfCount; y++)
            //    {
            //        const float objNearestLight = ShadowMaps.Sample(PointClamp, float3(shadowMapUV + float2(x, y) * texelSize, j)).r;
            //        if (depthFromLight > objNearestLight)
            //            total += 1.0f;
            //    }
            //}
		
            //total /= totalTexels;
            //float lightFactor = 1.0f - (total * depthFromLight);
		
			// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
			// to the light than this pixel - so the pixel gets no effect from this light
            if (depthFromLight < ShadowMaps.Sample(PointClamp, float3(shadowMapUV, j)).r)
            {
                const float light1Dist = length(gSpotLights[j].pos - input.worldPosition);
                float3 diffuseLight = gSpotLights[j].colour * max(dot(input.worldNormal, lightDir), 0) / light1Dist; // Equations from lighting lecture
                const float3 halfway = normalize(lightDir + cameraDirection);
                const float3 specularLight = diffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference
                //diffuseLight *= lightFactor;

                resDiffuse += diffuseLight;
                resSpecular += specularLight;
            }
        }
    }

    //for each dir light
    for (int k = 0; k < gDirLights[0].numLights && gDirLights[k].enabled; ++k)
    {
        const float3 lightDir = gDirLights[k].facing;
        
    	// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
		// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
		// These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
        const float4 viewPosition = mul(gDirLights[k].viewMatrix, float4(input.worldPosition, 1.0f));
        const float4 projection = mul(gDirLights[k].projMatrix, viewPosition);

		// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
		// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
        float2 shadowMapUV = 0.5f * projection.xy / projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y; // Check if pixel is within light cone

		// Get depth of this pixel if it were visible from the light (another advanced projection step)
        const float depthFromLight = projection.z / projection.w - depthAdjust; //*** Adjustment so polygons don't shadow themselves
		
		
        const float texelSize = 1.0f / 1024;
	
        float total = 0.0f;
	
        for (int x = -pcfCount; x <= pcfCount; x++)
        {
            for (int y = -pcfCount; y <= pcfCount; y++)
            {
                const float objNearestLight = ShadowMaps.Sample(PointClamp, float3(shadowMapUV + float2(x, y) * texelSize, k)).r;
                if (depthFromLight > objNearestLight)
                    total += 1.0f;
            }
        }
		
        total /= totalTexels;
		
        float lightFactor = 1.0f - (total * depthFromLight);
		
		
		// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		// to the light than this pixel - so the pixel gets no effect from this light
        if (depthFromLight < ShadowMaps.Sample(PointClamp, float3(shadowMapUV, k + gSpotLights[0].numLights)).r)
        {
            float3 diffuseLight = gDirLights[k].colour * max(dot(input.worldNormal, lightDir), 0); // Equations from lighting lecture
            const float3 halfway = normalize(lightDir + cameraDirection);
            const float3 specularLight = diffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference
            diffuseLight *= lightFactor;

            resDiffuse += diffuseLight;
            resSpecular += specularLight;
        }
    }
    
    //for each point light
    for (int l = 0; l < gPointLights[0].numLights && gPointLights[l].enabled; ++l)
    {
        const float3 lightDir = normalize(gPointLights[j].pos - input.worldPosition);
        
        for (int face = 0; face < 6; ++face)
        {
            
    	    // Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
		    // pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
		    // These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
            const float4 viewPosition = mul(gPointLights[l].viewMatrices[face], float4(input.worldPosition, 1.0f));
            const float4 projection = mul(gPointLights[l].projMatrix, viewPosition);

		    // Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
		    // Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
            float2 shadowMapUV = 0.5f * projection.xy / projection.w + float2(0.5f, 0.5f);
            shadowMapUV.y = 1.0f - shadowMapUV.y; // Check if pixel is within light cone

		    // Get depth of this pixel if it were visible from the light (another advanced projection step)
            const float depthFromLight = projection.z / projection.w - depthAdjust; //*** Adjustment so polygons don't shadow themselves
		
		
            //const float texelSize = 1.0f / 1024;
            //float total = 0.0f;
            //for (int x = -pcfCount; x <= pcfCount; x++)
            //{
            //    for (int y = -pcfCount; y <= pcfCount; y++)
            //    {
            //        const float objNearestLight = ShadowMaps.Sample(PointClamp, float3(shadowMapUV + float2(x, y) * texelSize, l)).r;
            //        if (depthFromLight > objNearestLight)
            //            total += 1.0f;
            //    }
            //}
		
            //total /= totalTexels;
            //float lightFactor = 1.0f - (total * depthFromLight);
		
            
		    // Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		    // to the light than this pixel - so the pixel gets no effect from this light
            if (depthFromLight < ShadowMaps.Sample(PointClamp, float3(shadowMapUV, l + gSpotLights[0].numLights + gDirLights[0].numLights + face)).r)
            {
                const float light1Dist = length(input.worldPosition - gSpotLights[j].pos);
                float3 diffuseLight = (gPointLights[l].colour * max(dot(input.worldNormal, lightDir), 0)) / light1Dist; // Equations from lighting lecture
                const float3 halfway = normalize(lightDir + cameraDirection);
                const float3 specularLight = diffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference
                //diffuseLight *= lightFactor;

                resDiffuse += diffuseLight;
                resSpecular += specularLight;
            }
        }
    }
    
    float4 diffuseIBL = 1;
    
    if (ambientMap.Sample(PointClamp, 0).r != 0.0)
    {
        diffuseIBL = (ambientMap.Sample(PointClamp, input.worldNormal)) * 2.0f;
    }
    
    
	////////////////////
	// Combine lighting and textures

    // Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    const float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv) * diffuseIBL;
    const float3 diffuseMaterialColour = textureColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    const float specularMaterialColour = textureColour.a; // Specular material colour in texture A (shininess of the surface)

    // Combine lighting with texture colours
    float3 finalColour = resDiffuse * diffuseMaterialColour + resSpecular * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}