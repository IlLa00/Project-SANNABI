#pragma once

class Animator;

class VFX
{
public:
    void Init(const string& key, const string& spriteSheetPath, int frameCount, float duration, bool loop);
    void Update(float deltaTime);
    void Render(HDC hdc);
    void Play(Vector position);
    void Stop();
    bool IsActive() const;

    void SetVFXType(const string& type);
    string GetVFXType() { return vfxType; }

private:
    string vfxType; 
    Animator* animator = nullptr;
    bool isActive = false;
    float currentDuration = 0.f;

    string _key;

    bool bFinsih = false;
    bool _loop;
};

