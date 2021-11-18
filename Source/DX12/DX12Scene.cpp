#include "DX12Scene.h"

CDX12Scene::CDX12Scene(CDX12Engine* engine, std::string fileName)
{

}

void CDX12Scene::InitTextures()
{
}

void CDX12Scene::RenderScene(float& frameTime)
{
}

void CDX12Scene::UpdateScene(float& frameTime)
{
}

void CDX12Scene::RenderSceneFromCamera(CCamera* camera)
{
}

ImTextureID CDX12Scene::GetTextureSRV()
{
    return ImTextureID();
}

void CDX12Scene::Save(std::string fileName)
{
}

void CDX12Scene::Resize(UINT newX, UINT newY) {
}

void CDX12Scene::PostProcessingPass()
{
}

void CDX12Scene::RenderToDepthMap()
{
}

void CDX12Scene::DisplayPostProcessingEffects()
{
}
