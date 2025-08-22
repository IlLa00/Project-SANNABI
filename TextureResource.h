#pragma once

class TextureResource
{
public:
	void Load(string fileName);
	void Load(wstring fileName);

	// 해당 리소스가 그려지는 부분
	void Render(HDC hdc, Vector pos, bool applyCamera = true);
	void Render(HDC hdc, int srcX, int srcY, int srcWidth, int srcHeight, Vector destPos, int destWidth, int destHeight);
	void RenderRotated(HDC hdc, int srcX, int srcY, int srcWidth, int srcHeight,
		Vector destPos, int destWidth, int destHeight,
		float rotationAngle, Vector rotationPivot, bool flip = false);

	int32 GetSizeX() { return _sizeX; }
	int32 GetSizeY() { return _sizeY; }

	void SetSizeX(int32 newValue) { _sizeX = newValue; }
	void SetSizeY(int32 newValue) { _sizeY = newValue; }

	void SetTransparent(int32 newValue) { _transparent = newValue; }

public:
	HDC		_textureHdc = 0;
	HBITMAP _bitmap;
	int32 _transparent = -1;	
	int32 _sizeX = 0;
	int32 _sizeY = 0;
	int32 _frameCountX = 0;
	int32 _frameCountY = 0;

	float _defaultScaleX = 1.0f; 
	float _defaultScaleY = 1.0f; 
};