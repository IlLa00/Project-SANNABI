#include "pch.h"
#include "SoundManager.h"
#include <algorithm>

bool SoundManager::Init(HWND hwnd)
{
    HRESULT hr = DirectSoundCreate8(NULL, &g_pDS, NULL);
    if (FAILED(hr))
        return false;

    hr = g_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
    if (FAILED(hr))
        return false;

    bgmPaused = false;
    currentBGM = "";

    return true;
}

void SoundManager::Update()
{
    if (!currentBGM.empty())
    {
        auto it = soundBuffers.find(currentBGM);
        if (it != soundBuffers.end())
        {
            DWORD status;
            it->second->GetStatus(&status);
            if (!(status & DSBSTATUS_PLAYING))
                currentBGM = "";
        }
    }
}

void SoundManager::Destroy()
{
    StopAllSounds();

    for (auto const& [key, val] : soundBuffers)
    {
        if (val)
        {
            val->Release();
        }
    }

    soundBuffers.clear();
    soundTypes.clear();

    if (g_pDS)
    {
        g_pDS->Release();
        g_pDS = nullptr;
    }
}

bool SoundManager::LoadSound(const string& key, const string& path, SoundType type)
{
    if (soundBuffers.find(key) != soundBuffers.end())
        return true; 

    IDirectSoundBuffer8* buffer = nullptr;
    if (LoadWaveFile(path, &buffer))
    {
        soundBuffers[key] = buffer;
        soundTypes[key] = type;
        return true;
    }

    return false;
}

void SoundManager::PlaySound(const string& key, bool isLooping)
{
    auto it = soundBuffers.find(key);
    if (it != soundBuffers.end())
    {
        it->second->SetCurrentPosition(0);
        DWORD dwFlags = isLooping ? DSBPLAY_LOOPING : 0;
        it->second->Play(0, 0, dwFlags);

        // BGM 트래킹
        auto typeIt = soundTypes.find(key);
        if (typeIt != soundTypes.end() && typeIt->second == SoundType::BGM)
        {
            currentBGM = key;
            bgmPaused = false;
        }
    }
}

void SoundManager::StopSound(const string& key)
{
    auto it = soundBuffers.find(key);
    if (it != soundBuffers.end())
    {
        it->second->Stop();

        // BGM 트래킹 업데이트
        if (currentBGM == key)
        {
            currentBGM = "";
            bgmPaused = false;
        }
    }
}

void SoundManager::StopAllSounds()
{
    for (auto& [key, buffer] : soundBuffers)
    {
        if (buffer)
            buffer->Stop();
    }

    currentBGM = "";
    bgmPaused = false;
}

void SoundManager::PlayBGM(const string& key, bool fadeIn)
{
    if (!currentBGM.empty())
        StopBGM();

    auto it = soundTypes.find(key);
    if (it != soundTypes.end() && it->second == SoundType::BGM)
        PlaySound(key, true);
}

void SoundManager::StopBGM(bool fadeOut)
{
    if (!currentBGM.empty())
        StopSound(currentBGM);
}

void SoundManager::PauseBGM()
{
    if (!currentBGM.empty() && !bgmPaused)
    {
        auto it = soundBuffers.find(currentBGM);
        if (it != soundBuffers.end())
        {
            it->second->Stop();
            bgmPaused = true;
        }
    }
}

void SoundManager::ResumeBGM()
{
    if (!currentBGM.empty() && bgmPaused)
    {
        auto it = soundBuffers.find(currentBGM);
        if (it != soundBuffers.end())
        {
            it->second->Play(0, 0, DSBPLAY_LOOPING);
            bgmPaused = false;
        }
    }
}

bool SoundManager::IsPlaying(const string& key)
{
    auto it = soundBuffers.find(key);
    if (it != soundBuffers.end())
    {
        DWORD status;
        it->second->GetStatus(&status);
        return (status & DSBSTATUS_PLAYING) != 0;
    }
    return false;
}

bool SoundManager::LoadWaveFile(const string& path, IDirectSoundBuffer8** ppBuffer)
{
    if (!g_pDS) return false;

    FILE* file = nullptr;
    fopen_s(&file, path.c_str(), "rb");
    if (!file)
        return false;

    WAVHeader header = {};

    if (!ReadWAVHeader(file, header))
    {
        fclose(file);
        return false;
    }

    BYTE* audioData = new BYTE[header.dataSize];
    size_t bytesRead = fread(audioData, 1, header.dataSize, file);
    fclose(file);

    if (bytesRead != header.dataSize)
    {
        delete[] audioData;
        return false;
    }

    bool result = CreateSoundBuffer(header, audioData, ppBuffer);

    delete[] audioData;
    return result;
}

bool SoundManager::ReadWAVHeader(FILE* file, WAVHeader& header)
{
    // RIFF 헤더 읽기
    fread(header.riff, 1, 4, file);
    if (strncmp(header.riff, "RIFF", 4) != 0)
        return false;

    fread(&header.fileSize, sizeof(DWORD), 1, file);

    fread(header.wave, 1, 4, file);
    if (strncmp(header.wave, "WAVE", 4) != 0)
        return false;

    // fmt 청크 찾기
    char chunkID[4];
    DWORD chunkSize;

    while (true)
    {
        if (fread(chunkID, 1, 4, file) != 4)
            return false;
        if (fread(&chunkSize, sizeof(DWORD), 1, file) != 1)
            return false;

        if (strncmp(chunkID, "fmt ", 4) == 0)
        {
            memcpy(header.fmt, chunkID, 4);
            header.fmtSize = chunkSize;

            fread(&header.audioFormat, sizeof(WORD), 1, file);
            fread(&header.numChannels, sizeof(WORD), 1, file);
            fread(&header.sampleRate, sizeof(DWORD), 1, file);
            fread(&header.byteRate, sizeof(DWORD), 1, file);
            fread(&header.blockAlign, sizeof(WORD), 1, file);
            fread(&header.bitsPerSample, sizeof(WORD), 1, file);

            if (chunkSize > 16)
            {
                fseek(file, chunkSize - 16, SEEK_CUR);
            }
            break;
        }
        else
        {
            fseek(file, chunkSize, SEEK_CUR);
        }
    }

    // data 청크 찾기
    while (true)
    {
        if (fread(chunkID, 1, 4, file) != 4)
            return false;
        if (fread(&chunkSize, sizeof(DWORD), 1, file) != 1)
            return false;

        if (strncmp(chunkID, "data", 4) == 0)
        {
            memcpy(header.data, chunkID, 4);
            header.dataSize = chunkSize;
            break;
        }
        else
        {
            fseek(file, chunkSize, SEEK_CUR);
        }
    }

    // 유효성 검사
    if (header.audioFormat != 1) // PCM이 아님
        return false;

    if (header.numChannels == 0 || header.numChannels > 2)
        return false;

    if (header.sampleRate == 0 || header.bitsPerSample == 0)
        return false;

    return true;
}

bool SoundManager::CreateSoundBuffer(const WAVHeader& header, BYTE* audioData, IDirectSoundBuffer8** buffer)
{
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = header.numChannels;
    wfx.nSamplesPerSec = header.sampleRate;
    wfx.wBitsPerSample = header.bitsPerSample;
    wfx.nBlockAlign = header.blockAlign;
    wfx.nAvgBytesPerSec = header.byteRate;
    wfx.cbSize = 0;

    DSBUFFERDESC dsbd = {};
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
    dsbd.dwBufferBytes = header.dataSize;
    dsbd.lpwfxFormat = &wfx;

    IDirectSoundBuffer* tempBuffer = nullptr;
    HRESULT hr = g_pDS->CreateSoundBuffer(&dsbd, &tempBuffer, nullptr);
    if (FAILED(hr))
        return false;

    hr = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)buffer);
    tempBuffer->Release();

    if (FAILED(hr))
        return false;

    LPVOID audioPtr1, audioPtr2;
    DWORD audioBytes1, audioBytes2;

    hr = (*buffer)->Lock(0, header.dataSize, &audioPtr1, &audioBytes1, &audioPtr2, &audioBytes2, 0);
    if (FAILED(hr))
    {
        (*buffer)->Release();
        *buffer = nullptr;
        return false;
    }

    memcpy(audioPtr1, audioData, audioBytes1);

    if (audioPtr2 != nullptr)
        memcpy(audioPtr2, audioData + audioBytes1, audioBytes2);

    (*buffer)->Unlock(audioPtr1, audioBytes1, audioPtr2, audioBytes2);

    return true;
}





