//--------------------------------------------------------------------------------------
// PBR Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader receives position and normal from the vertex shader and uses them to calculate
// lighting per pixel.
// Differs from the PBR shader for the not use of normal maps

#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D DiffuseSpecularMap : register(t0); // Textures here can contain a diffuse map (main colour) in their rgb channels and a specular map (shininess) in the a channel
Texture2D AOMap : register(t1); // Ambient Occlusion Map, precalculated by the artist, holds the amount of occlusion for each texel
Texture2D RoughnessMap : register(t4); // Roughness map, holds how the material is shiny or rough, this will modify the specular light
Texture2D MetalnessMap : register(t5); // Metalness Map, tipically holds 0 or 1, rarely inbetween.

SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic - this is the sampler used for the texture above

TextureCube IBLMap : register(t6);

Texture2D ShadowMaps[10] : register(t7);
SamplerState PointClamp : register(s1);

static const float PI = 3.14159265359f;

float3 CalculateLight(float3 lightPos, float lightIntensity, float3 colour, float3 diffuse, float3 specular, float3 n, float3 v, float3 worldPos, float roughness, float3 albedo)
{
    float nDotV = max(dot(n, v), 0.001f);
    
    // Get light vector (normal towards light from pixel), attenuate light intensity at the same time
    float3 l = lightPos - worldPos;
    float rdist = 1.0f / length(l);
    l *= rdist;
    float li = lightIntensity * rdist * rdist;
    float3 lc = colour;

    // Halfway vector (normal halfway between view and light vector)
    float3 h = normalize(l + v);

    // Various dot products used throughout
    float nDotL = max(dot(n, l), 0.001f);
    float nDotH = max(dot(n, h), 0.001f);
    float vDotH = max(dot(v, h), 0.001f);

    // Lambert diffuse
    float3 lambert = albedo / PI;

    // Microfacet specular - fresnel term
    float3 F = specular + (1 - specular) * pow(max(1.0f - vDotH, 0.0f), 5.0f);

    // Microfacet specular - normal distribution term
    float alpha = max(roughness * roughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
    float alpha2 = alpha * alpha;
    float nDotH2 = nDotH * nDotH;
    float dn = nDotH2 * (alpha2 - 1) + 1;
    float D = alpha2 / (PI * dn * dn);

    // Microfacet specular - geometry term
    float k = (roughness + 1);
    k = k * k / 8;
    float gV = nDotV / (nDotV * (1 - k) + k);
    float gL = nDotL / (nDotL * (1 - k) + k);
    float G = gV * gL;

    // Full brdf, diffuse + specular
    float3 brdf = lambert + F * G * D / (4 * nDotL * nDotV);

    // Accumulate punctual light equation for this light
    diffuse += PI * li * lc * brdf * nDotL;
    
    return diffuse;
}

    
static const float totalTexels = (gPcfSamples * 2.0f + 1.0f) * (gPcfSamples * 2.0f + 1.0f);

float PCF(float depthFromLight, float2 shadowMapUV, int i)
{
    
    
    //get the shadow map size
    float2 size = 0;
    ShadowMaps[i].GetDimensions(size.x, size.y);
    
    //store the texel size
    const float texelSize = 1.0f / size.x;
	
  //  float total = 0.0f;
    
  //  float dx = ddx(shadowMapUV);
  //  float dy = ddy(shadowMapUV);
    
  //  //go through the nearby pixels in the X and in the Y coordinates
  //  [fastopt]
  //  for (float x = -gPcfSamples; x <= gPcfSamples; x += 1.0)
  //  {
  //      [fastopt]
  //      for (float y = -gPcfSamples; y <= gPcfSamples; y += 1.0)
  //      {
  //          //get the depth value for that coordinate
  //          const float objNearestLight = ShadowMaps[i].Sample(PointClamp, shadowMapUV + float2(x, y) * texelSize).r;
            
  //          //if it less than the depth of the light
  //          if (depthFromLight > objNearestLight)
  //          //add one to the total variable
  //              total += 1.0f;
  //      }
  //  }
		////after the loops are done, make the average
  //  total /= totalTexels;
		
  //  //return the average multiplied by the depth form light inverted
  //  return 1.0f - (total * depthFromLight);
    
    float sum = 0;
    float x, y;

    float pcfPoints = 1.5;
    
    for (y = -pcfPoints; y <= pcfPoints; y += 1.0)
        for (x = -pcfPoints; x <= pcfPoints; x += 1.0)
            sum += ShadowMaps[i].Sample(PointClamp,shadowMapUV + float2(x, y) * texelSize);

    return sum / 16;
    
}

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(LightingPixelShaderInput input) : SV_Target
{
    // Normal might have been scaled by model scaling or interpolation so renormalise
    input.worldNormal = normalize(input.worldNormal);
    
    float3 resSpecular = 0.0f;
    
    ///////////////////////
    // Sample texture
    
    float3 albedo = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    
    float opacity = DiffuseSpecularMap.Sample(TexSampler, input.uv).a;
    
    if (!opacity)
        discard;
   
    
    float ao = 1;
    if(gHasAoMap) 
        ao = AOMap.Sample(TexSampler, input.uv).r;
    
    float metalness = 0;
    if (gHasMetallnessMap)
        metalness = MetalnessMap.Sample(TexSampler, input.uv).r;
    
    float roughness = gRoughness;
    if (gHasRoughnessMap)
        roughness = RoughnessMap.Sample(TexSampler, input.uv).r;
    
    // Direction from pixel to camera
    const float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);
    
	///////////////////////
    // Global illumination
    float nDotV = max(dot(input.worldNormal, cameraDirection), 0.001f);

    // Select specular color based on metalness
    float3 specularColour = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);

    // Reflection vector for sampling the cubemap for specular reflections
    float3 r = reflect(-cameraDirection, input.worldNormal);

    // Sample environment cubemap, use small mipmap for diffuse, use mipmap based on roughness for specular
    float3 diffuseIBL = IBLMap.Sample(TexSampler, r).rgb * 2.0f; // This approximation gives somewhat weak diffuse, so scale by 2
    float roughnessMip = 8 * log2(gRoughness + 1); // Heuristic to convert roughness to mip-map. Rougher surfaces will use smaller (blurrier) mip-maps
    float3 specularIBL = IBLMap.SampleLevel(TexSampler, r, roughnessMip).rgb;

    // Fresnel for IBL: when surface is at more of a glancing angle reflection of the scene increases
    float3 F_IBL = specularColour + (1 - specularColour) * pow(max(1.0f - nDotV, 0.0f), 5.0f);

    // Overall global illumination - rough approximation
    float3 resDiffuse = (albedo * diffuseIBL + (1 - gRoughness) * F_IBL * specularIBL) * ao;
    
    
	///////////////////////
	// Calculate lighting
    
	//// Lights ////
    
    for (int i = 0; i < gNumLights && gLights[i].enabled; ++i)
    {
        resDiffuse += CalculateLight(gLights[i].position, gLights[i].intensity, gLights[i].colour, resDiffuse, resSpecular, input.worldNormal, cameraDirection, input.worldPosition, gRoughness, albedo);
    }
    
	//for each spot light
    for (int j = 0; j < gNumSpotLights && gSpotLights[j].enabled; ++j)
    {
        const float3 lightDir = normalize(gSpotLights[j].pos - input.worldPosition);

    	//if the pixel is in the light cone
        if (dot(-gSpotLights[j].facing, lightDir) > gSpotLights[j].cosHalfAngle)
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
            
            // Bias slope
            float bias = gDepthAdjust * tan(acos(dot(input.worldNormal, lightDir)));
            bias = clamp(bias, 0, 0.01);
            
			// Get depth of this pixel if it were visible from the light (another advanced projection step)
            const float depthFromLight = projection.z / projection.w - bias; //*** Adjustment so polygons don't shadow themselves
            
		        // Calcluate pcf value   
            float PCFvalue = PCF(depthFromLight, shadowMapUV, j);
            
			// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
			// to the light than this pixel - so the pixel gets no effect from this light
            if (1 /*depthFromLight < ShadowMaps[j].Sample(PointClamp, float2(shadowMapUV)).r*/)
            {
                resDiffuse += CalculateLight(gSpotLights[j].pos, gSpotLights[j].intensity, gSpotLights[j].colour, resDiffuse, resSpecular, input.worldNormal, cameraDirection, input.worldPosition, gRoughness, DiffuseSpecularMap.Sample(TexSampler, input.uv).rgb) * PCFvalue;
            }
        }
    }

    //for each dir light
    for (int k = 0; k < gNumDirLights && gDirLights[k].enabled; ++k)
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
        
        // Bias Slope
        float bias = gDepthAdjust * tan(acos(dot(input.worldNormal, lightDir)));
        bias = clamp(bias, 0, 0.01);
        
		// Get depth of this pixel if it were visible from the light (another advanced projection step)
        const float depthFromLight = projection.z / projection.w - bias; //*** Adjustment so polygons don't shadow themselves
		
        float PCFValue = PCF(depthFromLight, shadowMapUV, k);
            
		// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		// to the light than this pixel - so the pixel gets no effect from this light
        if (1 /*depthFromLight < ShadowMaps[k /*+ numSpotLights].Sample(PointClamp, shadowMapUV).r*/)
        {
            float nDotV = max(dot(input.worldNormal, cameraDirection), 0.001f);
            
            float li = gDirLights[k].intensity;
            float3 l = gDirLights[k].facing;
            float3 lc = gDirLights[k].colour;

            // Halfway vector (normal halfway between view and light vector)
            float3 h = normalize(l + cameraDirection);

            // Various dot products used throughout
            float nDotL = max(dot(input.worldNormal, l), 0.001f);
            float nDotH = max(dot(input.worldNormal, h), 0.001f);
            float vDotH = max(dot(cameraDirection, h), 0.001f);

            // Lambert diffuse
            float3 lambert = albedo / PI;

            // Microfacet specular - fresnel term
            float3 F = resSpecular + (1 - resSpecular) * pow(max(1.0f - vDotH, 0.0f), 5.0f);

            // Microfacet specular - normal distribution term
            float alpha = max(gRoughness * gRoughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
            float alpha2 = alpha * alpha;
            float nDotH2 = nDotH * nDotH;
            float dn = nDotH2 * (alpha2 - 1) + 1;
            float D = alpha2 / (PI * dn * dn);

            // Microfacet specular - geometry term
            float k = (gRoughness + 1);
            k = k * k / 8;
            float gV = nDotV / (nDotV * (1 - k) + k);
            float gL = nDotL / (nDotL * (1 - k) + k);
            float G = gV * gL;

            // Full brdf, diffuse + specular
            float3 brdf = lambert + F * G * D / (4 * nDotL * nDotV);

            // Accumulate punctual light equation for this light
            resDiffuse += (PI * li * lc * brdf * nDotL) * PCFValue;
            
        }
    }
    
    //for each point light
    [fastopt]
    for (int l = 0; l < gNumPointLights && gPointLights[l].enabled; ++l)
    {
        const float3 lightDir = normalize(gPointLights[l].pos - input.worldPosition);
        
        [fastopt]
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
            
            ////Bias slope

            float bias = gDepthAdjust * tan(acos(dot(input.worldNormal, lightDir)));
            bias = clamp(bias, 0, 0.01);
            
		    // Get depth of this pixel if it were visible from the light (another advanced projection step)
            const float depthFromLight = projection.z / projection.w - bias; //*** Adjustment so polygons don't shadow themselves
             
		    // Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		    // to the light than this pixel - so the pixel gets no effect from this light
            
            float depth = ShadowMaps[l /*+ gNumSpotLights + gNumDirLights */ + face].Sample(PointClamp, shadowMapUV).r;
           
            float3 currDiffuse = CalculateLight(gPointLights[l].pos, gPointLights[l].intensity, gPointLights[l].colour, resDiffuse, resSpecular, input.worldNormal, cameraDirection, input.worldPosition, gRoughness, DiffuseSpecularMap.Sample(TexSampler, input.uv).rgb);
            
            float PCFValue = PCF(depthFromLight, shadowMapUV, l + face);
            
            if (depthFromLight > 0 && depthFromLight < depth)
            {
                resDiffuse += currDiffuse /** PCFValue*/;
            }
        }
    }

    return float4(resDiffuse, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}