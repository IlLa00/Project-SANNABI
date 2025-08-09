#include "pch.h"
#include "ResourceManager.h"
#include "TextureResource.h"
#include "Engine.h"

void ResourceManager::Init()
{
   
}

void ResourceManager::Destroy()
{
    for (auto iter : _textures)
        delete iter.second;
    
    _textures.clear();
}

TextureResource* ResourceManager::GetTexture(string meshName)
{
    auto iter = _textures.find(meshName);
    if (iter != _textures.end())
        return iter->second;
    
    return nullptr;
}

bool ResourceManager::HasTexture(const string& key) const
{
    return _textures.find(key) != _textures.end();
}

TextureResource* ResourceManager::LoadTextureFromPath(const string& key, const string& path,
    int32 transparent, int32 xFrameCount, int32 yFrameCount)
{
    if (HasTexture(key))
        return GetTexture(key);

    LoadTexture(key, path, transparent, xFrameCount, yFrameCount);

    return GetTexture(key);
}

void ResourceManager::LoadAnimationTexture(const string& key, const string& path,
    int frameCountX, int frameCountY, int32 transparent)
{
    if (HasTexture(key))
        return; 

    LoadTexture(key, path, transparent, frameCountX, frameCountY);
}

void ResourceManager::LoadTexture(string key, string texturePath, int32 transparent, int32 xFrameCount, int32 yFrameCount)
{
    if (HasTexture(key))
        return;

    TextureResource* texture = new TextureResource();
    {
        fs::path fullPath = fs::current_path() / L"Sprite/" / texturePath;
        fullPath += L".bmp";
        HDC hdc = ::GetDC(Engine::GetInstance()->GetHwnd());
        texture->_textureHdc = ::CreateCompatibleDC(hdc);
        texture->_bitmap = (HBITMAP)::LoadImageW(
            nullptr,
            fullPath.c_str(),
            IMAGE_BITMAP,
            0,
            0,
            LR_LOADFROMFILE | LR_CREATEDIBSECTION
        );

        if (texture->_bitmap == 0)
        {
            ::MessageBox(Engine::GetInstance()->GetHwnd(), fullPath.c_str(), L"Invalid Texture Load", MB_OK);
            delete texture;
            ::ReleaseDC(Engine::GetInstance()->GetHwnd(), hdc);
            return;
        }

        texture->_transparent = transparent;
        HBITMAP prev = (HBITMAP)::SelectObject(texture->_textureHdc, texture->_bitmap);
        ::DeleteObject(prev);

        BITMAP bit = {};
        ::GetObject(texture->_bitmap, sizeof(BITMAP), &bit);
        texture->_sizeX = bit.bmWidth;
        texture->_sizeY = bit.bmHeight;
        texture->_frameCountX = xFrameCount;
        texture->_frameCountY = yFrameCount;

        ::ReleaseDC(Engine::GetInstance()->GetHwnd(), hdc);
    }

    _textures[key] = texture;
}

