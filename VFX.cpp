#include "pch.h"
#include "VFX.h"
#include "VFXManager.h"
#include "Animator.h"

void VFX::Init(const string& key, const string& spriteSheetPath, int frameCount, float duration, bool loop)
{
    animator = new Animator();
    animator->Init(this);

    _key = key;
    _loop = loop;
    animator->AddAnimation(key, spriteSheetPath, frameCount, duration, loop);

}

void VFX::Update(float deltaTime)
{
    if (!isActive) return;

    if (animator->IsFinished() && _loop)
    {
        isActive = false;
        return;
    }

    animator->Update();
}

void VFX::Render(HDC hdc)
{
    if (!isActive) return;

    animator->Render(hdc, 1.f);
}

void VFX::Play(Vector position, float rotation)
{
    isActive = true;
    _position = position;
    _rotation = rotation;

    animator->PlayAnimation(_key, true);
}

void VFX::Stop()
{
    isActive = false;

    animator->StopAnimation();
}

bool VFX::IsActive() const
{
    return isActive;
}

void VFX::SetVFXType(const string& type)
{
    vfxType = type;
}