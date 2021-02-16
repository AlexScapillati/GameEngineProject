//--------------------------------------------------------------------------------------
// Texture Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader simply samples a diffuse texture map and tints with colours from vertex shader

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory (the words map and texture are used interchangeably)

Texture2D DiffuseSpecularMap : register(t0); // Diffuse map (main colour) in rgb and specular map (shininess level) in alpha - C++ must load this into slot 0
Texture2D AOMap : register(t1);
Texture2D DisplacementMap : register(t2); //WIP
Texture2D NormalHeightMap : register(t3); // Normal map in rgb and height maps in alpha - C++ must load this into slot 1
Texture2D RoughnessMap : register(t4);
SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic

Texture2DArray ShadowMaps : register(t5); //an array of shadow maps
SamplerState PointClamp : register(s1); //sampler for the shadow maps 


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

//***| INFO |*********************************************************************************
// Normal mapping pixel shader function. The lighting part of the shader is the same as the
// per-pixel lighting shader - only the source of the surface normal is different
//
// An extra "Normal Map" texture is used - this contains normal (x,y,z) data in place of
// (r,g,b) data indicating the normal of the surface *per-texel*. This allows the lighting
// to take account of bumps on the texture surface. Using these normals is complex:
//    1. We must store a "tangent" vector as well as a normal for each vertex (the tangent
//       is basically the direction of the texture U axis in model space for each vertex)
//    2. Get the (interpolated) model normal and tangent at this pixel from the vertex
//       shader - these are the X and Z axes of "tangent space"
//    3. Use a "cross-product" to calculate the bi-tangent - the missing Y axis
//    4. Form the "tangent matrix" by combining these axes
//    5. Extract the normal from the normal map texture for this pixel
//    6. Use the tangent matrix to transform the texture normal into model space, then
//       use the world matrix to transform it into world space
//    7. This final world-space normal can be used in the usual lighting calculations, and
//       will show the "bumpiness" of the normal map
//
// Note that all this detail boils down to just five extra lines of code here
//********************************************************************************************
float4 main(NormalMappingPixelShaderInput input) : SV_Target
{
	
	//************************
	// Normal Map Extraction
	//************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
    float3 modelNormal = normalize(input.modelNormal);
    float3 modelTangent = normalize(input.modelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
    float3 modelBiTangent = cross(modelNormal, modelTangent);
    const float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);
	

	//****| INFO |**********************************************************************************//
	// The following few lines are the parallax mapping. Converts the camera direction into model
	// space and adjusts the UVs based on that and the bump depth of the texel we are looking at
	// Although short, this code involves some intricate matrix work / space transformations
	//**********************************************************************************************//

	// Get normalised vector to camera for parallax mapping and specular equation (this vector was calculated later in previous shaders)
    const float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

	// Transform camera vector from world into model space. Need *inverse* world matrix for this.
	// Only need 3x3 matrix to transform vectors, to invert a 3x3 matrix we transpose it (flip it about its diagonal)
    const float3x3 invWorldMatrix = transpose((float3x3) gWorldMatrix);
    const float3 cameraModelDir = normalize(mul(invWorldMatrix, cameraDirection)); // Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
    const float3x3 tangentMatrix = transpose(invTangentMatrix);
    const float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix).xy;
	
	// Get the height info from the normal map's alpha channel at the given texture coordinate
	// Rescale from 0->1 range to -x->+x range, x determined by ParallaxDepth setting
    const float textureHeight = gParallaxDepth * (NormalHeightMap.Sample(TexSampler, input.uv).a - 0.5f);
	
	// Use the depth of the texture to offset the given texture coordinate - this corrected texture coordinate will be used from here on
    const float2 offsetTexCoord = input.uv + textureHeight * textureOffsetDir;


	//*******************************************

	//****| INFO |**********************************************************************************//
	// The above chunk of code is used only to calculate "offsetTexCoord", which is the offset in 
	// which part of the texture we see at this pixel due to it being bumpy. The remaining code is 
	// exactly the same as normal mapping, but uses offsetTexCoord instead of the usual input.uv
	//**********************************************************************************************//

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
    const float3 textureNormal = 2.0f * NormalHeightMap.Sample(TexSampler, offsetTexCoord).rgb - 1.0f; // Scale from 0->1 to -1->1

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
    const float3 worldNormal = normalize(mul((float3x3) gWorldMatrix, mul(textureNormal, invTangentMatrix)));


	///////////////////////
	// Calculate lighting
	
	//// Lights ////
    
    float3 resDiffuse = gAmbientColour;
    float3 resSpecular = 0.0f;
	
    for (int i = 0; i < gLights[0].numLights; ++i)
    {
        const float3 lightDir = normalize(gLights[i].position - input.worldPosition);
        const float lightDist = length(gLights[i].position - input.worldPosition);

        const float3 diffuse = gLights[i].colour * max(dot(worldNormal, lightDir), 0) / lightDist;
        const float3 halfWay = normalize(lightDir + cameraDirection);
        const float3 specular = diffuse * pow(max(dot(worldNormal, halfWay), 0), gSpecularPower);
        
        resDiffuse += diffuse;
        resSpecular += specular;
    }
    
	//calculate lighting from spot lights

    const float depthAdjust = 0.0005f;
    //const int pcfCount = 2;
    
    //const float totalTexels = (pcfCount * 2.0f + 1.0f) * (pcfCount * 2.0f + 1.0f);
	
	//for each dir light
    for (int j = 0; j < gSpotLights[0].numLights; ++j)
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

			// Get depth of this pixel if it were visible from the light (another advanced projection step)
            const float depthFromLight = projection.z / projection.w - depthAdjust; //*** Adjustment so polygons don't shadow themselves
		
		
            //const float texelSize = 1.0f / 1024;
	
            //float total = 0.0f;
	
            //for (int x = -pcfCount; x <= pcfCount; x++)
            //{
            //    for (int y = -pcfCount; y <= pcfCount; y++)
            //    {
            //        const float objNearestLight = ShadowMaps.Sample(PointClamp, float3(shadowMapUV + float2(x, y) * texelSize,i)).r;
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
                const float3 diffuseLight = gSpotLights[j].colour * max(dot(worldNormal, lightDir), 0) / light1Dist; // Equations from lighting lecture
                const float3 halfway = normalize(lightDir + cameraDirection);
                const float3 specularLight = diffuseLight * pow(max(dot(worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference
                //diffuseLight *= lightFactor;

                resDiffuse += diffuseLight;
                resSpecular += specularLight;
            }
        }
    }
    
    for (int k = 0; k < gDirLights[0].numLights; ++k)
    {
        const float3 lightDir = gDirLights[k].facing;
        
    	//Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
		//pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
		//These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
        const float4 viewPosition = mul(gDirLights[k].viewMatrix, float4(input.worldPosition, 1.0f));
        const float4 projection = mul(gDirLights[k].projMatrix, viewPosition);

		//Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
	    //Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
        float2 shadowMapUV = 0.5f * projection.xy / projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y; // Check if pixel is within light cone

		//Get depth of this pixel if it were visible from the light (another advanced projection step)
        const float depthFromLight = projection.z / projection.w - depthAdjust; //*** Adjustment so polygons don't shadow themselves
		
		
        //const float texelSize = 1.0f / 1024;
	
        //float total = 0.0f;
	
        //for (int x = -pcfCount; x <= pcfCount; x++)
        //{
        //    for (int y = -pcfCount; y <= pcfCount; y++)
        //    {
        //        const float objNearestLight = ShadowMaps.Sample(PointClamp, float3(shadowMapUV + float2(x, y) * texelSize,i)).r;
        //        if (depthFromLight > objNearestLight)
        //            total += 1.0f;
        //    }
        //}
		
        //total /= totalTexels;
		
        //float lightFactor = 1.0f - (total * depthFromLight);
		
		
		//Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		//to the light than this pixel - so the pixel gets no effect from this light
        if (depthFromLight < ShadowMaps.Sample(PointClamp, float3(shadowMapUV, k + gSpotLights[0].numLights)).r)
        {
            const float3 diffuseLight = gDirLights[k].colour * max(dot(worldNormal, lightDir), 0); // Equations from lighting lecture
            const float3 halfway = normalize(lightDir + cameraDirection);
            const float3 specularLight = diffuseLight * pow(max(dot(worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference
            
            //diffuseLight *= lightFactor;

            resDiffuse += diffuseLight;
            resSpecular += specularLight;
        }
    }
    
    
    // Sample diffuse material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    // Use offset texture coordinate from parallax mapping
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, offsetTexCoord);
    
	//Apply AO map (if any is loaded)
    if (AOMap.Sample(TexSampler, offsetTexCoord).r != 0)
    {
        textureColour = textureColour * AOMap.Sample(TexSampler, offsetTexCoord).r;
    }
    
    //apply roughness map (if any is loaded)
    if (RoughnessMap.Sample(TexSampler, offsetTexCoord).r != 0)
    {
        resSpecular = resSpecular * RoughnessMap.Sample(TexSampler, offsetTexCoord).r;
    }
    
    const float3 diffuseMaterialColour = textureColour.rgb;

    float3 finalColour = resDiffuse * diffuseMaterialColour +
                         resSpecular;
    
    if (DiffuseSpecularMap.Sample(TexSampler, offsetTexCoord).a < 0.5)
        discard;
    
    return float4(finalColour,1); // Always use 1.0f for alpha - no alpha blending in this lab
}