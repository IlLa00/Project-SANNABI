#pragma once

class Actor;
class TextureResource;
class SpriteAnimation;

class Animator
{
public:
	void Init(Actor* _owner);
	void Update();
	void Render(HDC hdc, float scale);
	void Destroy();

	void AddAnimation(const string& name, const string& spriteSheetPath,
		int frameCount, float duration, bool loop = true);
	void PlayAnimation(const string& name, bool forceRestart = false);
	string GetCurrentAnimationName() const;

	RECT GetCurrentFrameRect();
	TextureResource* GetCurrentSpriteSheet();
	bool IsFinished() const;

	void SetFlip(bool flip) { bflip = flip; }
	bool IsFlip() const { return bflip; }

private:
	Actor* owner;
	map<string, SpriteAnimation*> animations;
	SpriteAnimation* currentAnimation = nullptr;
	string currentAnimationName;

	float currentTime = 0.f;
	int currentFrameIndex = 0;
	bool bPlaying = false;

	bool bflip = false;
};

