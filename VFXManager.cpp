#include "pch.h"
#include "VFXManager.h"
#include "VFX.h"

void VFXManager::Init()
{
    
}

void VFXManager::Update(float deltaTime)
{
    for (VFX* vfx : activeVFXs)
        vfx->Update(deltaTime);

    activeVFXs.erase(
        remove_if(activeVFXs.begin(), activeVFXs.end(),
            [](VFX* vfx) { return !vfx->IsActive(); }),
        activeVFXs.end());
}

void VFXManager::Render(HDC hdc)
{
    for (VFX* vfx : activeVFXs)
        vfx->Render(hdc);
}

void VFXManager::Destroy()
{
    for (auto& pair : vfxPool)
    {
        for (VFX* vfx : pair.second)
            SAFE_DELETE(vfx);
    }

    vfxPool.clear();
    activeVFXs.clear();
}

void VFXManager::RegisterVFX(const string& key, const string& spriteSheetPath, int frameCount, float duration, bool loop)
{
    // 이미 있으면
    if (vfxPool.find(key) != vfxPool.end())
        return;

    // 5개 생성
    for (int i = 0; i < 5; ++i)
    {
        VFX* newVFX = new VFX;
        newVFX->Init(key, spriteSheetPath, frameCount, duration, loop);
        newVFX->SetVFXType(key);
        vfxPool[key].push_back(newVFX);
    }
}

void VFXManager::Play(const string& key, Vector position)
{
    VFX* vfxToPlay = FindAvailableVFX(key);

    if (vfxToPlay)
    {
        vfxToPlay->Play(position);

        // 활성화 목록에 추가 (중복 방지를 위해 확인 필요)
        if (find(activeVFXs.begin(), activeVFXs.end(), vfxToPlay) == activeVFXs.end())
            activeVFXs.push_back(vfxToPlay);
    }
}

void VFXManager::Stop(const string& key)
{
    for (VFX* vfx : activeVFXs)
    {
        if (vfx->GetVFXType() == key) 
            vfx->Stop();
    }
}

VFX* VFXManager::FindAvailableVFX(const string& key)
{
    if (vfxPool.find(key) == vfxPool.end())
        return nullptr;

    for (VFX* vfx : vfxPool[key])
    {
        if (!vfx->IsActive())
            return vfx;
    }
}