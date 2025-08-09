#include "pch.h"
#include "SpriteRenderComponent.h"
#include "Animator.h"
#include "Actor.h"

void SpriteRenderComponent::Init(Actor* _owner)
{
	Super::Init(_owner);

	animator = new Animator;
	animator->Init(_owner);
}

void SpriteRenderComponent::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (owner->GetDirection().x == 1)
		animator->SetFlip(false);
	else
		animator->SetFlip(true);

	if (animator)
		animator->Update();
}

void SpriteRenderComponent::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	float scale = GetOwner()->GetScale();

	if (animator)
		animator->Render(_hdcBack, scale);
}

void SpriteRenderComponent::Destroy()
{
	if (animator)
		SAFE_DELETE(animator);

	Super::Destroy();
}

void SpriteRenderComponent::AddAnimation(const string& name, const string& spriteSheetPath, int frameCount, float duration, bool loop)
{
	if (animator)
		animator->AddAnimation(name, spriteSheetPath, frameCount, duration, loop);
}

void SpriteRenderComponent::PlayAnimation(const string& name, bool forceRestart)
{
	if (animator)
		animator->PlayAnimation(name, forceRestart);
}

string SpriteRenderComponent::GetCurrentAnimationName() const
{
	if (animator)
		return animator->GetCurrentAnimationName();
	return "";
}
