#pragma once
#include "Singleton.h"
#include <dsound.h>
#include <map>
#include <string>

using namespace std;

struct WAVHeader
{
    char riff[4];           // "RIFF"
    DWORD fileSize;         // ���� ũ��
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    DWORD fmtSize;          // fmt ûũ ũ�� (���� 16)
    WORD audioFormat;       // ����� ���� (1 = PCM)
    WORD numChannels;       // ä�� �� (1 = ���, 2 = ���׷���)
    DWORD sampleRate;       // ���ø� ����Ʈ
    DWORD byteRate;         // ����Ʈ ����Ʈ
    WORD blockAlign;        // ��� ����
    WORD bitsPerSample;     // ���ô� ��Ʈ ��
    char data[4];           // "data"
    DWORD dataSize;         // ���� ����� ������ ũ��
};

enum class SoundType
{
    BGM,    // �������
    SFX     // ȿ����
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

    // ���� ���� �Լ���
    bool LoadWaveFile(const string& path, IDirectSoundBuffer8** ppBuffer);
    bool ReadWAVHeader(FILE* file, WAVHeader& header);
    bool CreateSoundBuffer(const WAVHeader& header, BYTE* audioData, IDirectSoundBuffer8** buffer);
};