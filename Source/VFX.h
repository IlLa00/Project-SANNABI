#pragma once

class Animator;

class VFX
{
public:
    void Init(const string& key, const string& spriteSheetPath, int frameCount, float duration, bool loop);
    void Update(float deltaTime);
    void Render(HDC hdc);
    void Play(Vector position, float rotation);
    void Stop();
    bool IsActive() const;

    void SetVFXType(const string& type);
    string GetVFXType() { return vfxType; }

    void SetRotation(float rotation) { _rotation = rotation; }
    float GetRotation() { return _rotation; }

    void SetPosition(Vector position) { _position = position; }
    Vector GetPosition() { return _position; }

private:
    string vfxType; 
    Animator* animator = nullptr;
    bool isActive = false;
    float currentDuration = 0.f;

    Vector _position = Vector(0, 0);
    float _rotation = 0.f;

    string _key;

    bool bFinsih = false;
    bool _loop;
};

