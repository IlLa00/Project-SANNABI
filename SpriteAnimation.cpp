#include "pch.h"
#include "SpriteAnimation.h"
#include "TextureResource.h"
#include "ResourceManager.h"

void SpriteAnimation::Init(const string& spriteSheetPath, int _frameCount, float _duration, bool _loop)
{
    frameCount = _frameCount;
    duration = _duration;
    bloop = _loop;

    textureKey = spriteSheetPath; 

    spriteSheet = ResourceManager::GetInstance()->LoadTextureFromPath(
        textureKey,
        spriteSheetPath,
        RGB(255, 255, 255), // 흰색을 투명화시키겠다
        _frameCount,
        1 // 기본적으로 가로 1줄로 배열
    );

    if (spriteSheet)
        SetupFrameData();
}

RECT SpriteAnimation::GetFrameRect(int frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= static_cast<int>(frameRects.size()))
        return { 0, 0, 0, 0 };

    return frameRects[frameIndex];
}

void SpriteAnimation::SetupFrameData()
{
    if (!spriteSheet || frameCount <= 0)
        return;

    int totalFramesX = spriteSheet->_frameCountX;
    int totalFramesY = spriteSheet->_frameCountY;

    frameWidth = spriteSheet->_sizeX / totalFramesX;
    frameHeight = spriteSheet->_sizeY / totalFramesY;

    CreateFrameRects();
}

void SpriteAnimation::CreateFrameRects()
{
    frameRects.clear();
    frameRects.reserve(frameCount);

    int frameCountX = spriteSheet->_frameCountX;
    int frameCountY = spriteSheet->_frameCountY;

    for (int i = 0; i < frameCount; ++i)
    {
        // 좌상단부터 우하단으로 순서대로 프레임 계산
        int x = i % frameCountX;
        int y = i / frameCountX;

        // 범위 체크
        if (y >= frameCountY)
            break;

        RECT frameRect;
        frameRect.left = x * frameWidth;
        frameRect.top = y * frameHeight;
        frameRect.right = frameRect.left + frameWidth;
        frameRect.bottom = frameRect.top + frameHeight;

        frameRects.push_back(frameRect);
    }
}