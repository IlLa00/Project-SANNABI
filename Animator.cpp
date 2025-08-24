#include "pch.h"
#include "Animator.h"
#include "TimerManager.h"
#include "SpriteAnimation.h"
#include "TextureResource.h"
#include "Actor.h"
#include "Component.h"

void Animator::Init(Actor* _owner, Component* _ownerComponent, bool IsVFX)
{
	if (_owner)
	{
		owner = _owner;
		ownerComponent = _ownerComponent;
	}

	bVFX = IsVFX;
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
			if (currentAnimation->IsLoop() && !bVFX)
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
	if (!currentAnimation)
		return;
	TextureResource* spriteSheet = currentAnimation->GetSpriteSheet();
	if (!spriteSheet)
		return;

	RECT frameRect = GetCurrentFrameRect();

	if (!owner && !ownerComponent) // VFX전용
	{
		Vector pos = VFXPostion;

		int originalWidth = frameRect.right - frameRect.left;
		int originalHeight = frameRect.bottom - frameRect.top;
		int scaledWidth = static_cast<int>(originalWidth * scale);
		int scaledHeight = static_cast<int>(originalHeight * scale);

		int renderWidth = scaledWidth;
		if (bflip)
			renderWidth = -scaledWidth;

		spriteSheet->Render(hdc,
			frameRect.left, frameRect.top,
			originalWidth, originalHeight,
			pos, renderWidth, scaledHeight);

		return;
	}
	
	Vector pos = ownerComponent->GetPosition();

	float rotation = ownerComponent->GetRotation();

	int originalWidth = frameRect.right - frameRect.left;
	int originalHeight = frameRect.bottom - frameRect.top;
	int scaledWidth = static_cast<int>(originalWidth * scale);
	int scaledHeight = static_cast<int>(originalHeight * scale);

	if (useRotation && rotationAngle != 0.0f)
	{
		spriteSheet->RenderRotated(hdc,
			frameRect.left, frameRect.top,
			originalWidth, originalHeight,
			pos, scaledWidth, scaledHeight,
			rotationAngle, rotationPivot, bflip);
	}
	else
	{
		int renderWidth = scaledWidth;
		if (bflip)
			renderWidth = -scaledWidth;

		spriteSheet->Render(hdc,
			frameRect.left, frameRect.top,
			originalWidth, originalHeight,
			pos, renderWidth, scaledHeight);
	}
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
	return !bPlaying;
}

void Animator::SetRotationInfo(bool bUse, float angle, Vector pivot)
{
	useRotation = bUse;
	rotationAngle = angle;
	rotationPivot = pivot;
}

void Animator::CalculateRotatePoints(POINT destPoint[3], int width, int height, Vector centerPos, float angle, Vector pivot)
{
	float cosA = cos(angle);
	float sinA = sin(angle);

	float pivotOffsetX = width * (pivot.x - 0.5f);
	float pivotOffsetY = height * (pivot.y - 0.5f);

	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;

	float x1 = -halfWidth - pivotOffsetX;
	float y1 = -halfHeight - pivotOffsetY;
	float x2 = halfWidth - pivotOffsetX;
	float y2 = -halfHeight - pivotOffsetY;
	float x3 = -halfWidth - pivotOffsetX;
	float y3 = halfHeight - pivotOffsetY;

	destPoint[0].x = centerPos.x + (x1 * cosA - y1 * sinA);
	destPoint[0].y = centerPos.y + (x1 * sinA + y1 * cosA);

	destPoint[1].x = centerPos.x + (x2 * cosA - y2 * sinA);
	destPoint[1].y = centerPos.y + (x2 * sinA + y2 * cosA);

	destPoint[2].x = centerPos.x + (x3 * cosA - y3 * sinA);
	destPoint[2].y = centerPos.y + (x3 * sinA + y3 * cosA);
}
