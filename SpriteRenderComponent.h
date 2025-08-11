#pragma once

#include "Component.h"

class Animator;

class SpriteRenderComponent : public Component
{
	using Super = Component;

public:
	void Init(Actor* _owner) override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	void AddAnimation(const string& name, const string& spriteSheetPath,
		int frameCount, float duration, bool loop = true);
	void PlayAnimation(const string& name, bool forceRestart = false);
	
	void SetScale(float value) { scale = scale * value; }

	string GetCurrentAnimationName() const;
	Animator* GetAnimator() const { return animator; }

private:
	Animator* animator;

	float scale;
};

