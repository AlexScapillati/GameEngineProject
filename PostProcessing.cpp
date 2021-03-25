//--------------------------------------------------------------------------------------
// Post Processing functions
// Complementary of the class CScene
//--------------------------------------------------------------------------------------

#pragma once
#include "Scene.h"
#include "Common.h"
#include "State.h"
#include "GameObject.h"
#include "DirLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "External\imgui\imgui.h"
#include "Plant.h"

//*******************************
//**** Post-processing shader DirectX objects
// These are also added to Shader.h
extern ID3D11PixelShader* gBurnPostProcess;
extern ID3D11PixelShader* gCopyPostProcess;
extern ID3D11PixelShader* gTintPostProcess;
extern ID3D11PixelShader* gSsaoPostProcess;
extern ID3D11PixelShader* gBloomPostProcess;
extern ID3D11PixelShader* gSpiralPostProcess;
extern ID3D11VertexShader* g2DQuadVertexShader;
extern ID3D11PixelShader* gDistortPostProcess;
extern ID3D11PixelShader* gGodRaysPostProcess;
extern ID3D11PixelShader* gSsaoLastPostProcess;
extern ID3D11PixelShader* gHeatHazePostProcess;
extern ID3D11PixelShader* gGreyNoisePostProcess;
extern ID3D11PixelShader* gBloomLastPostProcess;
extern ID3D11VertexShader* g2DPolygonVertexShader;
extern ID3D11PixelShader* gGaussionBlurPostProcess;
extern ID3D11PixelShader* gChromaticAberrationPostProcess;

void CScene::LoadPostProcessingImages()
{
	if (!LoadTexture("Noise.png", &mNoiseMap, &mNoiseMapSRV) ||
		!LoadTexture("Burn.png", &mBurnMap, &mBurnMapSRV) ||
		!LoadTexture("Distort.png", &mDistortMap, &mDistortMapSRV) ||
		!LoadTexture("randomNoise.jpg", &mRandomMap, &mRandomMapSRV))
	{
		throw std::runtime_error("Error loading post processing images");
	}
}

void CScene::ReleasePostProcessingShaders()
{
	if (mBurnMap)				mBurnMap->Release();		mBurnMap = nullptr;
	if (mNoiseMap)				mNoiseMap->Release();		mNoiseMap = nullptr;
	if (mBurnMapSRV)			mBurnMapSRV->Release();		mBurnMapSRV = nullptr;
	if (mDistortMap)			mDistortMap->Release();		mDistortMap = nullptr;
	if (mNoiseMapSRV)			mNoiseMapSRV->Release();	mNoiseMapSRV = nullptr;
	if (mRandomMapSRV)			mRandomMapSRV->Release();   mRandomMapSRV = nullptr;
	if (mLuminanceMap)			mLuminanceMap->Release();   mLuminanceMap = nullptr;
	if (mDistortMapSRV)			mDistortMapSRV->Release();  mDistortMapSRV = nullptr;
	if (mRandomMap)				mRandomMap->Release();		mRandomMap = nullptr;
}

// Select the appropriate shader plus any additional textures required for a given post-process
// Helper function shared by full-screen, area and polygon post-processing functions below
void CScene::SelectPostProcessShaderAndTextures(PostProcess postProcess)
{
	switch (postProcess)
	{
	case CScene::PostProcess::None:
		break;

	case CScene::PostProcess::Copy:

		gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::Tint:

		gD3DContext->PSSetShader(gTintPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::GreyNoise:

		gD3DContext->PSSetShader(gGreyNoisePostProcess, nullptr, 0);

		// The noise offset is randomised to give a constantly changing noise effect (like tv static)
		gPostProcessingConstants.noiseOffset = { Random(0.0f, 1.0f), Random(0.0f, 1.0f) };

		// Give pixel shader access to the noise texture
		gD3DContext->PSSetShaderResources(1, 1, &mNoiseMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);

		break;

	case CScene::PostProcess::Burn:

		gD3DContext->PSSetShader(gBurnPostProcess, nullptr, 0);

		// Give pixel shader access to the burn texture (basically a height map that the burn level ascends)
		gD3DContext->PSSetShaderResources(1, 1, &mBurnMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);

		break;

	case CScene::PostProcess::Distort:

		gD3DContext->PSSetShader(gDistortPostProcess, nullptr, 0);

		// Give pixel shader access to the distortion texture (containts 2D vectors (in R & G) to shift the texture UVs to give a cut-glass impression)
		gD3DContext->PSSetShaderResources(1, 1, &mDistortMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);

		break;

	case CScene::PostProcess::Spiral:

		gD3DContext->PSSetShader(gSpiralPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::HeatHaze:

		gD3DContext->PSSetShader(gHeatHazePostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::ChromaticAberration:

		gD3DContext->PSSetShader(gChromaticAberrationPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::GaussionBlur:

		gD3DContext->PSSetShader(gGaussionBlurPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::SSAO:

		//****| INFO |*******************************************************************************************//
		//	SSAO Pass
		//	For the SSao pass we are going to do 4 passes
		//	The first we calculate the amount of occlusion and store it in the SSaoMap
		//	Second pass, optional, we blur the resulting texture
		//	Third pass, optional, we copy the finaltexture to the ssaomap
		//  Fourth pass, we multiply the ssao map and the scene texture together
		//*******************************************************************************************************//

		// 1st pass - Calculate the ssao amount

		gD3DContext->PSSetSamplers(1,1,&gPointSampler);

		gD3DContext->OMSetRenderTargets(1, &mSsaoMapRTV, nullptr);

		gD3DContext->PSSetShader(gSsaoPostProcess, nullptr, 0);

		gD3DContext->PSSetShaderResources(1, 1, &mFinalDepthStencilSRV);

		gD3DContext->PSSetShaderResources(2, 1, &mRandomMapSRV);

		gD3DContext->Draw(4, 0);

		// 2nd pass / Optional pass - Blur the occlusion map

		if (mSsaoBlur)
		{
			// Unbind the ssao map from the shader
			ID3D11ShaderResourceView* nullSRV = nullptr;
			gD3DContext->PSSetShaderResources(1, 1, &nullSRV);

			gD3DContext->OMSetRenderTargets(1, &mFinalRTV, mDepthStencilRTV);

			gD3DContext->PSSetShaderResources(0, 1, &mSsaoMapSRV);

			gD3DContext->PSSetShader(gGaussionBlurPostProcess, nullptr, 0);

			gD3DContext->Draw(4, 0);

			// Unbind the ssao map from the shader
			gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

			// 3nd pass - The ssao Map gets copied from the FinalRenderTarget

			gD3DContext->OMSetRenderTargets(1, &mSsaoMapRTV, mDepthStencilRTV);

			gD3DContext->PSSetShaderResources(0, 1, &mFinalTextureSRV);

			gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);

			gD3DContext->Draw(4, 0);

			// Unbind the final texture map from the shader
			gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
		}

		// 4th pass - the scene texture and the ssao map are blended toghether

		gD3DContext->OMSetRenderTargets(1, &mFinalRTV, nullptr);

		gD3DContext->PSSetShaderResources(0, 1, &mSceneSRV);

		gD3DContext->PSSetShaderResources(1, 1, &mSsaoMapSRV);

		gD3DContext->PSSetShader(gSsaoLastPostProcess, nullptr, 0);

		break;

	case CScene::PostProcess::Bloom:
	{
		//****| INFO |*******************************************************************************************//
		//	For the bloom post processing effect there are multiple passes, exactly 4.
		//	Fist the scene texture gets analysed and a luminance map gets created. This map is contains only the brightest zones of the scene.
		//	Secondly the luminance map gets blurried, using the Gaussian blur.
		//	Since the luminance render target and the luminance map are connected, in this pass the render target is the mFinalRenderTarget
		//	Thirdly there is a copy pass, the finalTexture content gets copyed to the luminance map
		//	Finally the luminance map and the scene texture are added together.
		//*******************************************************************************************************//

		// 1st pass - The luminance map gets extracted
		gD3DContext->OMSetRenderTargets(1, &mLuminanceRTV, nullptr);

		gD3DContext->PSSetShader(gBloomPostProcess, nullptr, 0);

		gD3DContext->Draw(4, 0);

		// 2nd pass - The luminance map gets blurried (rendered in the mFinalMap)
		gD3DContext->OMSetRenderTargets(1, &mFinalRTV, mDepthStencilRTV);

		gD3DContext->PSSetShaderResources(0, 1, &mLuminanceMapSRV);

		gD3DContext->PSSetShader(gGaussionBlurPostProcess, nullptr, 0);

		gD3DContext->Draw(4, 0);

		// Unbind the luminance map from the shader input so we can bind it to the render target
		ID3D11ShaderResourceView* nullSRV = nullptr;
		gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

		// 3rd pass - the luminance pass gets copied from the FinalTexture in the mLuminanceMap
		gD3DContext->OMSetRenderTargets(1, &mLuminanceRTV, mDepthStencilRTV);

		gD3DContext->PSSetShaderResources(0, 1, &mFinalTextureSRV);

		gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);

		gD3DContext->Draw(4, 0);

		gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

		// 4th pass - the scene texture and the luminance map are added toghether

		gD3DContext->OMSetRenderTargets(1, &mFinalRTV, nullptr);

		gD3DContext->PSSetShaderResources(0, 1, &mSceneSRV);

		gD3DContext->PSSetShaderResources(1, 1, &mLuminanceMapSRV);

		gD3DContext->PSSetShader(gBloomLastPostProcess, nullptr, 0);
	}
	break;

	case CScene::PostProcess::GodRays:

		// Here the light screen position must be calculated
		CVector3 lightPos;
		CVector3 lightScreenPos;

		if (!GOM->mDirLights.empty()) lightPos = GOM->mDirLights[0]->Position();
		else if (!GOM->mLights.empty()) lightPos = GOM->mLights[0]->Position();
		else if (!GOM->mSpotLights.empty()) lightPos = GOM->mSpotLights[0]->Position();
		else if (!GOM->mPointLights.empty()) lightPos = GOM->mPointLights[0]->Position();
		else break;

		// Convert the world point to screen space
		lightScreenPos = mCamera->PixelFromWorldPt(lightPos, mViewportX, mViewportY);

		if (lightScreenPos.z < 0.0f) break;

		// Scale it in uv coordinates (-1,1)
		lightScreenPos = ScaleBetween(lightScreenPos, -1.0f, 1.0f, { 0.0f,0.0f,0.0f }, { float(mViewportX),float(mViewportY),0.0f });

		// Set the point to the post processing buffer
		gPostProcessingConstants.lightScreenPos = { lightScreenPos.x, lightScreenPos.y };

		//Set the shader
		gD3DContext->PSSetShader(gGodRaysPostProcess, nullptr, 0);

		break;

	}
}

void CScene::FullScreenPostProcess(PostProcess postProcess)
{
	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &mFinalRTV, mDepthStencilRTV);

	// Give the pixel shader (post-processing shader) access to the scene texture
	gD3DContext->PSSetShaderResources(0, 1, &mSceneSRV);

	// Use point sampling (no bilinear, trilinear, mip-mapping etc. for most post-processes)
	gD3DContext->PSSetSamplers(0, 1, &gPointSampler);

	// Using special vertex shader that creates its own data for a 2D screen quad
	gD3DContext->VSSetShader(g2DQuadVertexShader, nullptr, 0);
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  // Switch off geometry shader when not using it (pass nullptr for first parameter)

	// States - no blending, write to depth buffer and ignore back-face culling
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);

	// No need to set vertex/index buffer (see 2D quad vertex shader), just indicate that the quad will be created as a triangle strip
	gD3DContext->IASetInputLayout(NULL); // No vertex data
	gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Set 2D area for full-screen post-processing (coordinates in 0->1 range)
	gPostProcessingConstants.area2DTopLeft = { 0, 0 }; // Top-left of entire screen
	gPostProcessingConstants.area2DSize = { 1, 1 }; // Full size of screen
	gPostProcessingConstants.area2DDepth = 0;        // Depth buffer value for full screen is as close as possible

													 // Pass over the above post-processing settings (also the per-process settings prepared in UpdateScene function below)
	UpdatePostProcessingConstBuffer(gPostProcessingConstBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);

	// Select shader and textures needed for the required post-processes (helper function above)
	SelectPostProcessShaderAndTextures(postProcess);

	// Draw a quad
	gD3DContext->Draw(4, 0);

	//These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
	ID3D11ShaderResourceView* nullSRV = nullptr;
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

	//now that the post processing is applied
	//set back the main target view

	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &mSceneRTV, mDepthStencilRTV);

	//set the copy shader
	gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);

	//set the final texture as a shader resource
	gD3DContext->PSSetShaderResources(0, 1, &mFinalTextureSRV);

	//draw a quad
	gD3DContext->Draw(4, 0);

	//unbind the texture from the pixel shader
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
}

// Perform an area post process from "scene texture" to back buffer at a given point in the world, with a given size (world units)
void CScene::AreaPostProcess(PostProcess postProcess, CVector3 worldPoint, CVector2 areaSize)
{
	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &mFinalRTV, mDepthStencilRTV);

	// Give the pixel shader (post-processing shader) access to the scene texture
	gD3DContext->PSSetShaderResources(0, 1, &mSceneSRV);

	// First perform a full-screen copy of the scene to back-buffer
	FullScreenPostProcess(PostProcess::Copy);

	// Now perform a post-process of a portion of the scene to the back-buffer (overwriting some of the copy above)
	// Note: The following code relies on many of the settings that were prepared in the FullScreenPostProcess call above, it only
	//       updates a few things that need to be changed for an area process. If you tinker with the code structure you need to be
	//       aware of all the work that the above function did that was also preparation for this post-process area step

	// Select shader/textures needed for required post-process
	SelectPostProcessShaderAndTextures(postProcess);

	// Enable alpha blending - area effects need to fade out at the edges or the hard edge of the area is visible
	// A couple of the shaders have been updated to put the effect into a soft circle
	// Alpha blending isn't enabled for fullscreen and polygon effects so it doesn't affect those (except heat-haze, which works a bit differently)
	gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);

	// Use picking methods to find the 2D position of the 3D point at the centre of the area effect
	auto worldPointTo2D = mCamera->PixelFromWorldPt(worldPoint, mViewportX, mViewportY);
	CVector2 area2DCentre = { worldPointTo2D.x, worldPointTo2D.y };
	float areaDistance = worldPointTo2D.z;

	// Nothing to do if given 3D point is behind the camera
	if (areaDistance < mCamera->NearClip())  return;

	// Convert pixel coordinates to 0->1 coordinates as used by the shader
	area2DCentre.x /= mViewportX;
	area2DCentre.y /= mViewportY;

	// Using new helper function here - it calculates the world space units covered by a pixel at a certain distance from the camera.
	// Use this to find the size of the 2D area we need to cover the world space size requested
	CVector2 pixelSizeAtPoint = mCamera->PixelSizeInWorldSpace(areaDistance, mViewportX, mViewportY);
	CVector2 area2DSize = { areaSize.x / pixelSizeAtPoint.x, areaSize.y / pixelSizeAtPoint.y };

	// Again convert the result in pixels to a result to 0->1 coordinates
	area2DSize.x /= mViewportX;
	area2DSize.y /= mViewportY;

	// Send the area top-left and size into the constant buffer - the 2DQuad vertex shader will use this to create a quad in the right place
	gPostProcessingConstants.area2DTopLeft = area2DCentre - 0.5f * area2DSize; // Top-left of area is centre - half the size
	gPostProcessingConstants.area2DSize = area2DSize;

	// Manually calculate depth buffer value from Z distance to the 3D point and camera near/far clip values. Result is 0->1 depth value
	// We've never seen this full calculation before, it's occasionally useful. It is derived from the material in the Picking lecture
	// Having the depth allows us to have area effects behind normal objects
	gPostProcessingConstants.area2DDepth = mCamera->FarClip() * (areaDistance - mCamera->NearClip()) / (mCamera->FarClip() - mCamera->NearClip());
	gPostProcessingConstants.area2DDepth /= areaDistance;

	// Pass over this post-processing area to shaders (also sends the per-process settings prepared in UpdateScene function below)
	UpdatePostProcessingConstBuffer(gPostProcessingConstBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);

	// Draw a quad
	gD3DContext->Draw(4, 0);

	//These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
	ID3D11ShaderResourceView* nullSRV = nullptr;
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

	//now that the post processing is applied
	//set back the main target view

	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &mSceneRTV, mDepthStencilRTV);

	//set the copy shader
	gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);

	//set the final texture as a shader resource
	gD3DContext->PSSetShaderResources(0, 1, &mFinalTextureSRV);

	//draw a quad
	gD3DContext->Draw(4, 0);

	//unbind the texture from the pixel shader
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
}

// Perform an post process from "scene texture" to back buffer within the given four-point polygon and a world matrix to position/rotate/scale the polygon
void CScene::PolygonPostProcess(PostProcess postProcess, const std::array<CVector3, 4>& points, const CMatrix4x4& worldMatrix)
{
	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &mFinalRTV, mDepthStencilRTV);

	// Give the pixel shader (post-processing shader) access to the scene texture
	gD3DContext->PSSetShaderResources(0, 1, &mSceneSRV);

	// First perform a full-screen copy of the scene to back-buffer
	FullScreenPostProcess(PostProcess::Copy);

	// Now perform a post-process of a portion of the scene to the back-buffer (overwriting some of the copy above)
	// Note: The following code relies on many of the settings that were prepared in the FullScreenPostProcess call above, it only
	//       updates a few things that need to be changed for an area process. If you tinker with the code structure you need to be
	//       aware of all the work that the above function did that was also preparation for this post-process area step

	// Select shader/textures needed for required post-process
	SelectPostProcessShaderAndTextures(postProcess);

	// Loop through the given points, transform each to 2D (this is what the vertex shader normally does in most labs)
	for (unsigned int i = 0; i < points.size(); ++i)
	{
		CVector4 modelPosition = CVector4(points[i], 1);
		CVector4 worldPosition = modelPosition * worldMatrix;
		CVector4 viewportPosition = worldPosition * mCamera->ViewProjectionMatrix();

		gPostProcessingConstants.polygon2DPoints[i] = viewportPosition;
	}

	// Pass over the polygon points to the shaders (also sends the per-process settings prepared in UpdateScene function below)
	UpdatePostProcessingConstBuffer(gPostProcessingConstBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstBuffer);

	// Select the special 2D polygon post-processing vertex shader and draw the polygon
	gD3DContext->VSSetShader(g2DPolygonVertexShader, nullptr, 0);

	gD3DContext->Draw(4, 0);

	//These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
	ID3D11ShaderResourceView* nullSRV = nullptr;
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

	//now that the post processing is applied
	//set back the main target view

	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &mSceneRTV, mDepthStencilRTV);

	//set the copy shader
	gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);

	//set the final texture as a shader resource
	gD3DContext->PSSetShaderResources(0, 1, &mFinalTextureSRV);

	//draw a quad
	gD3DContext->Draw(4, 0);

	//unbind the texture from the pixel shader
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
}

void CScene::PostProcessingPass()
{
	//show the postprocessing window
	if (!gViewportFullscreen)
	{
		//display the post processing effects in a window
		DisplayPostProcessingEffects();

		//iterate through the effects array
		auto filter = mPostProcessingFilters.begin();

		while (filter != mPostProcessingFilters.end())
		{
			// Run any post-processing steps
			if (filter->type != PostProcess::None)
			{
				if (filter->mode == PostProcessMode::Fullscreen)
				{
					FullScreenPostProcess(filter->type);
				}
				else if (filter->mode == PostProcessMode::Area)
				{
					//select an object using ImGui
					static CGameObject* select = nullptr;

					//select the object with ImGui
					if (select == nullptr)
					{
						if (ImGui::Begin("Select Object"))
						{
							for (auto obj : GOM->mObjects)
							{
								if (ImGui::Button(obj->GetName().c_str()))
								{
									select = obj;
								}
							}
						}
						ImGui::End();
					}
					else
					{
						static CVector3 areaPos = { 0,0,0 };
						static CVector2 areaSize = { 0,0 };

						ImGui::DragFloat3("Postion", areaPos.GetValuesArray());
						ImGui::DragFloat2("Size", areaSize.GetValuesArray());

						// Pass a 3D point for the centre of the affected area and the size of the (rectangular) area in world units
						AreaPostProcess(filter->type, areaPos, areaSize);

						//display the selected obj name and a button to change it
						auto selectedText = "Selected: " + select->GetName();
						ImGui::Text(selectedText.c_str());
						ImGui::SameLine();
						if (ImGui::Button("Change"))
						{
							select = nullptr;
						}
					}
				}
				else if (filter->mode == PostProcessMode::Polygon)
				{
					static CGameObject* select = nullptr;

					//select the object with ImGui
					if (select == nullptr)
					{
						if (ImGui::Begin("Select Object"))
						{
							for (auto obj : GOM->mObjects)
							{
								if (ImGui::Button(obj->GetName().c_str()))
								{
									select = obj;
								}
							}
						}
						ImGui::End();
					}
					else
					{
						//create a window to let the user modify the dimentions
						if (ImGui::Begin("Edit Volume"))
						{
							// An array of four points in world space - a tapered square centred at the origin
							static std::array<CVector3, 4> points = { {{-5,5,0}, {-5,-5,0}, {5,5,0}, {5,-5,0} } };

							ImGui::Separator();
							ImGui::Text("Edit Area");
							ImGui::DragFloat3("Top Left", points[0].GetValuesArray());
							ImGui::DragFloat3("Bottom Left", points[1].GetValuesArray());
							ImGui::DragFloat3("Top Right", points[2].GetValuesArray());
							ImGui::DragFloat3("Bottom Right", points[3].GetValuesArray());
							ImGui::Separator();

							static CMatrix4x4 polyMatrix = MatrixTranslation({ select->Position() });

							// Pass an array of 4 points and a matrix. Only supports 4 points.
							PolygonPostProcess(filter->type, points, polyMatrix);

							//display the selected obj name and a button to change it
							auto selectedText = "Selected: " + select->GetName();
							ImGui::Text(selectedText.c_str());
							ImGui::SameLine();
							if (ImGui::Button("Change"))
							{
								select = nullptr;
							}
						}
						ImGui::End();
					}
				}

				//These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
				ID3D11ShaderResourceView* nullSRV = nullptr;
				gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

				if (filter->type == PostProcess::SSAO ||
					filter->type == PostProcess::Bloom ||
					filter->type == PostProcess::Distort ||
					filter->type == PostProcess::Burn ||
					filter->type == PostProcess::GaussionBlur ||
					filter->type == PostProcess::GreyNoise ||
					filter->type == PostProcess::Spiral)
					gD3DContext->PSSetShaderResources(1, 1, &nullSRV);

				filter++;
			}
		}
	}
}

void CScene::RenderToDepthMap()
{
	// Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
	// Also clear the the shadow map depth buffer to the far distance
	gD3DContext->OMSetRenderTargets(0, nullptr, mFinalDepthStencilRTV);
	gD3DContext->ClearDepthStencilView(mFinalDepthStencilRTV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set cull none
	gD3DContext->RSSetState(gCullNoneState);

	//render just the objects that can cast shadows
	for (auto it : GOM->mObjects)
	{
		//basic geometry rendered, that means just render the model's geometry, leaving all the fancy shaders

		// Leave out the plants // TODO: since they have a opacity map, the depth map will get the model geometry 
		if (dynamic_cast<CPlant*>(it))
			return;
		else
			it->Render(true);
	}
}

void CScene::DisplayPostProcessingEffects()
{
	//-------------------------------------------------------
	// Post Processing Effects Window
	//-------------------------------------------------------

	static bool choosePP = false;
	static bool editProperties = false;

	//Render a window with all the postprocessing
	if (ImGui::Begin("PostProcessing", 0, ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		//show a button "new"
		if (ImGui::Button("New"))
		{
			choosePP = true;
		}

		auto filter = mPostProcessingFilters.begin();

		//display every ppfilter with a button
		while (filter != mPostProcessingFilters.end())
		{
			//****| INFO |*******************************************************************************************//
			//	The next variable is for the buttons management in ImGui.
			//	If there are multiple button witht the same lable, imgui will not recognise them.
			//	This problem is being avoided with the "position" of the filter in the list, hence using the std::distance function,
			//	added it to the button lable. The hashtags are used to not show the id in the GUI.
			//	The name of the filter can be used. However, if the user will input two of the same filters,
			//	than the id of the button(lable) would be the same.
			//*******************************************************************************************************//

			char buttonId = std::distance(mPostProcessingFilters.begin(), filter);

			if (ImGui::Button(mPostProcessStrings[(int)filter->type].c_str()))
			{
				//if the button is clicked, you can edit its properties
				editProperties = true;
			}

			if (mPostProcessingFilters.size() > 1)
			{
				if (filter != --mPostProcessingFilters.end())
				{
					ImGui::SameLine();

					std::string label = "Move Down##";

					label += (buttonId);

					if (ImGui::Button(label.c_str()))
					{
						std::iter_swap(++filter, filter);
					}
				}

				if (filter != mPostProcessingFilters.begin())
				{
					ImGui::SameLine();

					std::string label = "Move Up##";

					label += (buttonId);

					if (ImGui::Button(label.c_str()))
					{
						std::iter_swap(--filter, filter);
					}
				}
			}

			ImGui::SameLine();

			std::string label = "Remove##";

			label += (buttonId);

			if (ImGui::Button(label.c_str()))
			{
				filter = mPostProcessingFilters.erase(filter);
			}
			else
				filter++;
		}

		ImGui::End();
	}

	//-------------------------------------------------------
	// Post Processing Choose Window
	//-------------------------------------------------------

	//open a window that lets the user choose the post processing filter
	if (choosePP)
	{
		if (ImGui::Begin("Choose"))
		{
			static PostProcessFilter newPostProcess;

			std::string items = "";

			//populate the string with post processing items separated by \0
			for (auto s : mPostProcessStrings)
			{
				if (s == "Copy") continue;
				items += s + '\0';
			}

			static int selectType = 0;

			//display a menu with the PP types
			if (ImGui::Combo("Type", &selectType, items.c_str()))
			{
				//assign the mode of the new pp to the selected type ( + 1 because the Copy type is only used internally)
				newPostProcess.type = PostProcess(selectType + 1);
			}

			static int selectMode = 0;
			std::string modes = "";

			//populate the string with post processing modes separated by \0
			for (auto s : mPostProcessModeStrings)
			{
				modes += s + '\0';
			}

			//display a menu with the PP modes
			if (ImGui::Combo("Mode", &selectMode, modes.c_str()))
			{
				//assign the mode of the new pp to the selected mode
				newPostProcess.mode = PostProcessMode(selectMode);
			}

			//display a button for creating the pp effect
			if (ImGui::Button("OK"))
			{
				//if the new pp is not None or Copy
				if (newPostProcess.type != PostProcess::None && newPostProcess.type != PostProcess::Copy)
				{
					//push it back to the list
					mPostProcessingFilters.push_back(newPostProcess);

					//close the window
					choosePP = false;
				}
			}
		}
		ImGui::End();
	}

	//-------------------------------------------------------
	// Post Processing Properties Window
	//-------------------------------------------------------

	// Noise scaling adjusts how fine the grey noise is.
	static float grainSize = 140; // Fineness of the noise grain
	static bool showLuminanceMap = false;
	static bool showDepthMap = false;

	//open a window that lets the user modify the post processing properties
	if (editProperties && mPostProcessingFilters.size())
	{
		if (ImGui::Begin("PostProcessing Properties", &editProperties))
		{
			//for each effect
			for (auto postProcess : mPostProcessingFilters)
			{
				// Display the name of the effect 
				ImGui::Separator();
				ImGui::Text(mPostProcessStrings[int(postProcess.type)].c_str());
				ImGui::Separator();

				//go through the type and display the properties
				switch (postProcess.type)
				{
				case CScene::PostProcess::None:
					break;

				case CScene::PostProcess::Copy:
					break;

				case CScene::PostProcess::Tint:

					ImGui::ColorEdit3("Pick colour", gPostProcessingConstants.tintColour.GetValuesArray());

					break;

				case CScene::PostProcess::GreyNoise:

					ImGui::DragFloat("Grain size", &grainSize, 1.0f, 0.0f);

					//set the texture noise scale
					gPostProcessingConstants.noiseScale = { mViewportX / grainSize, mViewportY / grainSize };

					//The noise strength (default is 0.5)
					ImGui::DragFloat("Noise Strength", &gPostProcessingConstants.noiseStrength, 0.001f, 0.0f, 1.0f, "%.4f");

					//the distance between the centre of the texture and the beginning of the edge
					ImGui::DragFloat("Edge Distance", &gPostProcessingConstants.noiseEdge, 0.01f, 0.0f, 1.0f, "%.4f");

					break;

				case CScene::PostProcess::Burn:
					break;

				case CScene::PostProcess::Distort:

					ImGui::DragFloat("Distorsion Level", &gPostProcessingConstants.distortLevel, 0.001f);

					break;

				case CScene::PostProcess::Spiral:
					break;

				case CScene::PostProcess::HeatHaze:

					ImGui::DragFloat("Strength", &gPostProcessingConstants.heatEffectStrength, 0.0001f);

					ImGui::SliderFloat("Soft Edge", &gPostProcessingConstants.heatSoftEdge, 0.001f, 0.25f);

					break;

				case CScene::PostProcess::ChromaticAberration:

					ImGui::DragFloat("Amount", &gPostProcessingConstants.caAmount, 0.001f);
					ImGui::DragFloat("Soft Edge", &gPostProcessingConstants.caEdge, 0.001f);
					ImGui::DragFloat("Falloff", &gPostProcessingConstants.caFalloff, 0.001f);

					break;

				case CScene::PostProcess::GaussionBlur:

					ImGui::DragFloat("Directions", &gPostProcessingConstants.blurDirections, 0.1f, 0.01f, 64.0f);
					ImGui::DragFloat("Quality", &gPostProcessingConstants.blurQuality, 0.1, 1.0f, 64.0f);
					ImGui::DragFloat("Size", &gPostProcessingConstants.blurSize, 0.1f);

					break;

				case CScene::PostProcess::SSAO:

					//enable blur

					ImGui::Checkbox("Enable SSAO blur", &mSsaoBlur);
					ImGui::Checkbox("Show Depth Map", &showDepthMap);



					ImGui::DragFloat("Strength", &gPostProcessingConstants.ssaoStrenght, 0.01f, 0.0f, 10.0f);
					ImGui::DragFloat("Falloff", &gPostProcessingConstants.ssaoFalloff, 0.000000001f, 0.0f, 0.1f, "%.10f");
					ImGui::DragFloat("Area", &gPostProcessingConstants.ssaoArea, 0.001f, 0.0f, 1.0f, "%.5f");
					ImGui::DragFloat("Radius", &gPostProcessingConstants.ssaoRadius, 0.00001f, 0.0f, 0.001f, "%.10f");

					ImGui::Separator();

					if (mSsaoBlur)
					{
						ImGui::DragFloat("Directions", &gPostProcessingConstants.blurDirections, 0.1f, 0.01f, 64.0f);
						ImGui::DragFloat("Quality", &gPostProcessingConstants.blurQuality, 0.1, 1.0f, 64.0f);
						ImGui::DragFloat("Size", &gPostProcessingConstants.blurSize, 0.1f);
					}

					if (showDepthMap)
					{
						ImGui::Image(mSsaoMapSRV, { 256, 256});
					}

					break;

				case CScene::PostProcess::Bloom:

					ImGui::DragFloat("Threshold", &gPostProcessingConstants.bloomThreshold, 0.001f, 0.f, 3.0f);

					//for debugging porpurse
					//show a button to show the luminance map
					if (ImGui::Button("Show luminance texture"))
					{
						showLuminanceMap = !showLuminanceMap;
					}

					if (showLuminanceMap)
					{
						auto avaliableDim = ImGui::GetContentRegionAvail();
						ImGui::Image(mLuminanceMapSRV, avaliableDim);
					}

					//for the blur part of the bloom
					ImGui::DragFloat("Directions", &gPostProcessingConstants.blurDirections, 0.1f, 0.01f, 64.0f);
					ImGui::DragFloat("Quality", &gPostProcessingConstants.blurQuality, 0.1, 1.0f, 64.0f);
					ImGui::DragFloat("Size", &gPostProcessingConstants.blurSize, 0.1f);

					break;

				case CScene::PostProcess::GodRays:

					
					ImGui::DragFloat2("debug",gPostProcessingConstants.lightScreenPos.GetValuesArray());

					// Display the other settings
					ImGui::DragFloat("Weight", &gPostProcessingConstants.weight,0.001f,0.0f,1.0f);
					ImGui::DragFloat("Decay", &gPostProcessingConstants.decay,0.001f,0.0f,1.0f);
					ImGui::DragFloat("Exposure", &gPostProcessingConstants.exposure,0.001f,0.0f,1.0f);
					ImGui::DragFloat("Density", &gPostProcessingConstants.density,0.001f,0.0f,1.0f);
					ImGui::DragInt("Samples", &gPostProcessingConstants.numSamples,1,0.0f,MAXUINT16);

					break;
				}
			}

			if (ImGui::Button("OK"))
			{
				editProperties = false;
			}
		}
		ImGui::End();
	}
}