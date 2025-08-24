#pragma once

#include "Singleton.h"

class SpriteRenderComponent;
class Animator;
class VFX;

class VFXManager : public Singleton<VFXManager>
{
    friend Singleton<VFXManager>;

private:
    VFXManager() {}

public:
    void Init();
    void Update(float deltaTime);
    void Render(HDC hdc);
    void Destroy();

    void RegisterVFX(const string& key, const string& spriteSheetPath, int frameCount, float duration, bool loop);

    void Play(const string& key, Vector position);
    void Stop(const string& key);

private:
    map<string, vector<VFX*>> vfxPool;
    vector<VFX*> activeVFXs; // 렌더링을 위한 활성화된 VFX 목록

    VFX* FindAvailableVFX(const string& key);
};

