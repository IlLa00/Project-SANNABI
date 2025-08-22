#pragma once

#include "Component.h"

class Animator;

enum class ETransformMode
{
	FollowOwner, // 오너를 따라감
	Relative // 오너기준 상대적인 위치
};

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
	
	void SetTransformMode(ETransformMode newMode) { transformMode = newMode; }
	void SetScale(float value) { scale = scale * value; }

	string GetCurrentAnimationName() const;
	Animator* GetAnimator() const { return animator; }

	void SetUseRotation(bool newState) { bUseRotation = newState; }
	void SetRotationPivot(Vector pivot) { rotationPivot = pivot; }
	bool GetUseRotation() const { return bUseRotation; }
	Vector GetRotationPivot() const { return rotationPivot; }

private:
	Animator* animator;
	float scale;
	float rotationAngle = 0.f;

	ETransformMode transformMode = ETransformMode::FollowOwner;

	bool bUseRotation = false;
	Vector rotationPivot = { 0.5f,0.5f };
};

