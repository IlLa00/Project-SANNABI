#include "pch.h"
#include "TextureResource.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "Engine.h"

void TextureResource::Load(string fileName)
{
	// WinAPI 텍스처 로딩하는 방법
	{
		fs::path fullPath = fs::current_path();
		fullPath += "\\Lobby\\" + fileName + ".bmp";

		HDC hdc = ::GetDC(Engine::GetInstance()->GetHwnd());

		_textureHdc = ::CreateCompatibleDC(hdc);
		_bitmap = (HBITMAP)::LoadImageW(
			nullptr,
			fullPath.c_str(),
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION
		);
		if (_bitmap == 0)
		{
			::MessageBox(Engine::GetInstance()->GetHwnd(), fullPath.c_str(), L"Invalid Texture Load", MB_OK);
			return;
		}

		_transparent = RGB(255, 255, 255);

		HBITMAP prev = (HBITMAP)::SelectObject(_textureHdc, _bitmap);
		::DeleteObject(prev);

		BITMAP bit = {};
		::GetObject(_bitmap, sizeof(BITMAP), &bit);

		_sizeX = bit.bmWidth;
		_sizeY = bit.bmHeight;
	}
}

void TextureResource::Load(wstring fileName)
{
	HDC hdc = ::GetDC(Engine::GetInstance()->GetHwnd());

	_textureHdc = ::CreateCompatibleDC(hdc);
	_bitmap = (HBITMAP)::LoadImageW(
		nullptr,
		fileName.c_str(),
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION
	);

	if (_bitmap == 0)
	{
		::MessageBox(Engine::GetInstance()->GetHwnd(), fileName.c_str(), L"Invalid Texture Load", MB_OK);
		// 여기에 디버깅을 위한 출력 추가
		OutputDebugStringW(L"Failed to load: ");
		OutputDebugStringW(fileName.c_str());
		OutputDebugStringW(L"\n");
		return;
	}

	_transparent = RGB(255, 255, 255);

	HBITMAP prev = (HBITMAP)::SelectObject(_textureHdc, _bitmap);
	::DeleteObject(prev);

	BITMAP bit = {};
	::GetObject(_bitmap, sizeof(BITMAP), &bit);

	_sizeX = bit.bmWidth;
	_sizeY = bit.bmHeight;
}

void TextureResource::Render(HDC hdc, Vector pos, bool applyCamera)
{
	Vector screenPos = applyCamera ? CameraManager::GetInstance()->ConvertScreenPos(pos) : pos;

	if (_transparent == -1)
	{
		::BitBlt(hdc,	// 백버퍼에
			screenPos.x - (_sizeX / 2),	// 텍스처를 중심좌표로 그리기위해 size의 절반만큼 빼준다.
			screenPos.y - (_sizeY / 2),
			_sizeX,
			_sizeY,
			_textureHdc,	// 텍스쳐 그리기
			0,
			0,
			SRCCOPY);
	}
	else
	{
		::TransparentBlt(hdc,
			screenPos.x - (_sizeX / 2),
			screenPos.y - (_sizeY / 2),
			_sizeX,
			_sizeY,
			_textureHdc,
			0,
			0,
			_sizeX,
			_sizeY,
			_transparent);	// 어떤색상을 투명하게 그릴까
	}

}

void TextureResource::Render(HDC hdc, int srcX, int srcY, int srcWidth, int srcHeight, Vector destPos, int destWidth, int destHeight)
{
	_transparent = RGB(255, 255, 255);

	int renderPosX = (int32)destPos.x - (abs(destWidth) / 2);
	int renderPosY = (int32)destPos.y - (destHeight / 2);

	Vector renderPos = Vector(renderPosX, renderPosY);
	Vector screenPos = CameraManager::GetInstance()->ConvertScreenPos(renderPos);

	HDC tempHdc = ::CreateCompatibleDC(hdc);
	HBITMAP tempBitmap = ::CreateCompatibleBitmap(hdc, abs(destWidth), destHeight);
	HBITMAP oldBitmap = (HBITMAP)::SelectObject(tempHdc, tempBitmap);

	int finalSrcX = srcX;
	int finalSrcWidth = srcWidth;

	if (destWidth < 0)
	{
		finalSrcX = srcX + srcWidth;
		finalSrcWidth = -srcWidth;
	}

	::StretchBlt(tempHdc,
		0,
		0,
		abs(destWidth),
		destHeight,
		_textureHdc,
		finalSrcX,
		srcY,
		finalSrcWidth,
		srcHeight,
		SRCCOPY);

	::TransparentBlt(hdc,
		screenPos.x,
		screenPos.y,
		abs(destWidth),
		destHeight,
		tempHdc,
		0,
		0,
		abs(destWidth),
		destHeight,
		_transparent);

	::SelectObject(tempHdc, oldBitmap);
	::DeleteObject(tempBitmap);
	::DeleteDC(tempHdc);
}

void TextureResource::RenderRotated(HDC hdc, int srcX, int srcY, int srcWidth, int srcHeight, Vector destPos, int destWidth, int destHeight, float rotationAngle, Vector rotationPivot, bool flip)
{
	COLORREF transparent = RGB(255, 255, 255);
	Vector screenPos = CameraManager::GetInstance()->ConvertScreenPos(destPos);

	int finalSrcX = srcX;
	int finalSrcWidth = srcWidth;
	int finalDestWidth = abs(destWidth);

	if (flip || destWidth < 0)
	{
		finalSrcX = srcX + srcWidth;
		finalSrcWidth = -srcWidth;
	}

	// 임시 회전된 이미지를 만들기 위한 HDC
	HDC rotatedHdc = ::CreateCompatibleDC(hdc);

	// 회전 후 필요한 크기 계산
	float diagonal = sqrt(finalDestWidth * finalDestWidth + destHeight * destHeight);
	int rotatedSize = (int)ceil(diagonal);

	HBITMAP rotatedBitmap = ::CreateCompatibleBitmap(hdc, rotatedSize, rotatedSize);
	HBITMAP oldRotatedBitmap = (HBITMAP)::SelectObject(rotatedHdc, rotatedBitmap);

	// 배경을 투명색으로 채우기
	HBRUSH transparentBrush = ::CreateSolidBrush(transparent);
	RECT bgRect = { 0, 0, rotatedSize, rotatedSize };
	::FillRect(rotatedHdc, &bgRect, transparentBrush);
	::DeleteObject(transparentBrush);

	// 회전 변환 매트릭스 설정
	int oldMode = ::SetGraphicsMode(rotatedHdc, GM_ADVANCED);

	XFORM xform;
	float cosA = cos(rotationAngle);
	float sinA = sin(rotationAngle);

	xform.eM11 = cosA;
	xform.eM12 = sinA;
	xform.eM21 = -sinA;
	xform.eM22 = cosA;
	xform.eDx = rotatedSize / 2.0f;
	xform.eDy = rotatedSize / 2.0f;

	::SetWorldTransform(rotatedHdc, &xform);

	// 원본 이미지를 회전된 HDC에 그리기
	::StretchBlt(rotatedHdc, -finalDestWidth / 2, -destHeight / 2, finalDestWidth, destHeight,
		_textureHdc, finalSrcX, srcY, finalSrcWidth, srcHeight, SRCCOPY);

	// 변환 리셋
	::ModifyWorldTransform(rotatedHdc, nullptr, MWT_IDENTITY);
	::SetGraphicsMode(rotatedHdc, oldMode);

	// TransparentBlt로 투명화 처리하며 최종 출력
	::TransparentBlt(hdc,
		screenPos.x - rotatedSize / 2, screenPos.y - rotatedSize / 2,
		rotatedSize, rotatedSize,
		rotatedHdc, 0, 0, rotatedSize, rotatedSize,
		transparent);

	// 리소스 정리
	::SelectObject(rotatedHdc, oldRotatedBitmap);
	::DeleteObject(rotatedBitmap);
	::DeleteDC(rotatedHdc);
}
