#pragma once

class TextureResource;

class Button
{
public:
    void Init(Vector position, const wchar_t* text);
    void Update();
    void Render(HDC hdc);

    bool IsMouseOver();
    bool IsClicked();

    void SetActive(bool newState) { bActive = newState; }
    bool GetActive() { return bActive; }

private:
    TextureResource* normalTexture;
    TextureResource* hoverTexture;
    TextureResource* currentTexture;

    RECT buttonRect;
    Vector _position;

    bool bActive = false;

    const wchar_t* buttonText;
};

