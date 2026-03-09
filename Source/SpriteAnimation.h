#pragma once

class TextureResource;

class SpriteAnimation
{
public:
    void Init(const string& spriteSheetPath, int frameCount, float duration, bool loop = true);
    void Destroy();

    TextureResource* GetSpriteSheet() const { return spriteSheet; }
    int GetFrameCount() const { return frameCount; }
    float GetDuration() const { return duration; }
    bool IsLoop() const { return bloop; }

    RECT GetFrameRect(int frameIndex) const;

private:
    void CreateFrameRects();
    void SetupFrameData();

private:
    TextureResource* spriteSheet;
    vector<RECT> frameRects;
    int frameCount;
    float duration;
    bool bloop;

    int frameWidth;
    int frameHeight;
    string textureKey;
};

