#pragma once

#include "Singleton.h"

class TextureResource;

class ResourceManager : public Singleton<ResourceManager>
{
	friend Singleton<ResourceManager>;

private:
	ResourceManager() {}

public:
	void Init();
	void Destroy();	

	TextureResource* GetTexture(string meshName);
	void LoadTexture(string key, string texturePath, int32 transparent, int32 xFrameCount = 1, int32 yFrameCount = 1);
	
	TextureResource* LoadTextureFromPath(const string& key, const string& path,
		int32 transparent = RGB(255, 0, 255),
		int32 xFrameCount = 1, int32 yFrameCount = 1);

	bool HasTexture(const string& key) const;

	// 애니메이션 전용 로드 함수
	void LoadAnimationTexture(const string& key, const string& path,
		int frameCountX, int frameCountY = 1,
		int32 transparent = RGB(255, 0, 255));

private:
	unordered_map<string, TextureResource*> _textures;
};

