#pragma once

class Actor;
class Component;
class TextureResource;
class SpriteAnimation;

class Animator
{
public:
	void Init(Actor* _owner, Component* _ownerComponent, bool IsVFX = false);
	void Update();
	void Render(HDC hdc, float scale);
	void Destroy();

	void AddAnimation(const string& name, const string& spriteSheetPath,
		int frameCount, float duration, bool loop = true);
	void PlayAnimation(const string& name, bool forceRestart = false);
	void StopAnimation();
	string GetCurrentAnimationName() const;

	RECT GetCurrentFrameRect();
	TextureResource* GetCurrentSpriteSheet();
	bool IsFinished() const;

	void SetFlip(bool flip) { bflip = flip; }
	bool IsFlip() const { return bflip; }

	void SetRotationInfo(bool bUse, float angle, Vector pivot);

	void SetVFXPosition(Vector newPostion) { VFXPostion = newPostion; }
	Vector GetVFXPosition() { return VFXPostion; }

private:
	void CalculateRotatePoints(POINT destPoint[3], int width, int height, Vector centerPos, float angle, Vector pivot);

private:
	Actor* owner;
	Component* ownerComponent;

	map<string, SpriteAnimation*> animations;
	SpriteAnimation* currentAnimation = nullptr;
	string currentAnimationName;

	float currentTime = 0.f;
	int currentFrameIndex = 0;
	bool bPlaying = false;

	bool bflip = false;

	bool useRotation = false;
	float rotationAngle = 0.0f;
	Vector rotationPivot = { 0.5f,0.5f };

	Vector VFXPostion;
	bool bVFX = false;
};

