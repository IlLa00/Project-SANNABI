#include "pch.h"
#include "Animator.h"
#include "TimerManager.h"
#include "SpriteAnimation.h"
#include "TextureResource.h"
#include "Actor.h"

void Animator::Init(Actor* _owner)
{
	if (_owner)
		owner = _owner;
}

void Animator::Update()
{
	if (!currentAnimation || !bPlaying)
		return;

	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	currentTime += deltaTime;

	float frameDuration = currentAnimation->GetDuration() / currentAnimation->GetFrameCount();

	if (currentTime >= frameDuration)
	{
		currentTime = 0.0f;
		currentFrameIndex++;

		if (currentFrameIndex >= currentAnimation->GetFrameCount())
		{
			if (currentAnimation->IsLoop())
				currentFrameIndex = 0;
			else
			{
				currentFrameIndex = currentAnimation->GetFrameCount() - 1;
				bPlaying = false; // 애니메이션 종료
			}
		}
	}
}

void Animator::Render(HDC hdc, float scale)
{
	if (!currentAnimation || !owner)
		return;

	TextureResource* spriteSheet = currentAnimation->GetSpriteSheet();
	if (!spriteSheet)
		return;

	RECT frameRect = GetCurrentFrameRect();
	Vector pos = owner->GetPosition();

	int originalWidth = frameRect.right - frameRect.left;
	int originalHeight = frameRect.bottom - frameRect.top;

	int scaledWidth = static_cast<int>(originalWidth * scale);
	int scaledHeight = static_cast<int>(originalHeight * scale);

	Vector finalRenderPos = pos;
	int renderWidth = scaledWidth;

	if (bflip)
	{
		renderWidth = -scaledWidth;
		// 반전 시 위치를 수동으로 보정하는 이 코드를 제거합니다.
		// finalRenderPos.x += scaledWidth; 
	}

	spriteSheet->Render(hdc,
		frameRect.left,
		frameRect.top,
		originalWidth,
		originalHeight,
		finalRenderPos,
		renderWidth,
		scaledHeight
	);
}

void  Animator::Destroy()
{
	for (auto& pair : animations)
		SAFE_DELETE(pair.second);

	animations.clear();
}

void Animator::AddAnimation(const string& name, const string& spriteSheetPath,
	int frameCount, float duration, bool loop)
{
	if (animations.find(name) != animations.end())
		return; // 이미 존재하는 애니메이션

	SpriteAnimation* animation = new SpriteAnimation;
	animation->Init(spriteSheetPath, frameCount, duration, loop);

	animations[name] = animation;
}
void Animator::PlayAnimation(const string& name, bool forceRestart)
{
	auto it = animations.find(name);
	if (it == animations.end())
		return; // 존재하지 않는 애니메이션

	if (currentAnimationName == name && !forceRestart)
		return; // 이미 재생 중

	if (bPlaying) 
		StopAnimation();

	currentAnimation = it->second;
	currentAnimationName = name;
	currentTime = 0.0f;
	currentFrameIndex = 0;
	bPlaying = true;
}

void Animator::StopAnimation()
{
	bPlaying = false;
}

string Animator::GetCurrentAnimationName() const
{
	return currentAnimationName;
}

RECT Animator::GetCurrentFrameRect()
{
	if (!currentAnimation)
		return { 0, 0, 0, 0 };

	return currentAnimation->GetFrameRect(currentFrameIndex);
}

TextureResource* Animator::GetCurrentSpriteSheet()
{
	if (!currentAnimation)
		return nullptr;

	return currentAnimation->GetSpriteSheet();
}

bool Animator::IsFinished() const
{
	return bPlaying;
}