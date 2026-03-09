#pragma once
#include "Singleton.h"
#include <dsound.h>
#include <map>
#include <string>

using namespace std;

struct WAVHeader
{
    char riff[4];           // "RIFF"
    DWORD fileSize;         // 파일 크기
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    DWORD fmtSize;          // fmt 청크 크기 (보통 16)
    WORD audioFormat;       // 오디오 포맷 (1 = PCM)
    WORD numChannels;       // 채널 수 (1 = 모노, 2 = 스테레오)
    DWORD sampleRate;       // 샘플링 레이트
    DWORD byteRate;         // 바이트 레이트
    WORD blockAlign;        // 블록 정렬
    WORD bitsPerSample;     // 샘플당 비트 수
    char data[4];           // "data"
    DWORD dataSize;         // 실제 오디오 데이터 크기
};

enum class SoundType
{
    BGM,    // 배경음악
    SFX     // 효과음
};

class SoundManager : public Singleton<SoundManager>
{
    friend Singleton<SoundManager>;
private:
    SoundManager() : g_pDS(nullptr), bgmPaused(false) {}

public:
    bool Init(HWND hwnd);
    void Update();
    void Destroy();

    bool LoadSound(const string& key, const string& path, SoundType type);
    void PlaySound(const string& key, bool isLooping = false);
    void StopSound(const string& key);
    void StopAllSounds();

    void PlayBGM(const string& key, bool fadeIn = false);
    void StopBGM(bool fadeOut = false);
    void PauseBGM();
    void ResumeBGM();

    bool IsPlaying(const string& key);
    bool IsBGMPlaying() const { return !currentBGM.empty(); }

private:
    IDirectSound8* g_pDS;
    map<string, IDirectSoundBuffer8*> soundBuffers;
    map<string, SoundType> soundTypes;

    string currentBGM;
    bool bgmPaused;

    // 내부 헬퍼 함수들
    bool LoadWaveFile(const string& path, IDirectSoundBuffer8** ppBuffer);
    bool ReadWAVHeader(FILE* file, WAVHeader& header);
    bool CreateSoundBuffer(const WAVHeader& header, BYTE* audioData, IDirectSoundBuffer8** buffer);
};