#include "SDLSoundSystem.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>

namespace std
{
    template<>
    struct hash<dae::SoundId>
    {
        size_t operator()(dae::SoundId id) const noexcept
        {
            return hash<int>{}(static_cast<int>(id));
        }
    };
}

namespace dae
{
    struct SoundRequest
    {
        SoundId id;
        int volume;
    };

    struct SDLSoundSystem::Impl
    {
        Impl()
            : m_Mixer(nullptr)
            , m_Quit(false)
            , m_Muted(false)
        {
            if (!MIX_Init())
            {
                SDL_Log("MIX_Init failed: %s", SDL_GetError());
                return;
            }

            m_Mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            if (!m_Mixer)
            {
                SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
                MIX_Quit();
                return;
            }

            MIX_SetMixerGain(m_Mixer, 0.3f);

            m_WorkerThread = std::thread(&Impl::ProcessQueue, this);
        }

        ~Impl()
        {
            {
                std::lock_guard<std::mutex> lock(m_QueueMutex);
                m_Quit = true;
            }
            m_Condition.notify_one();

            if (m_WorkerThread.joinable())
                m_WorkerThread.join();

            for (auto& [id, audio] : m_AudioMap)
                MIX_DestroyAudio(audio);
            m_AudioMap.clear();

            if (m_Mixer)
            {
                MIX_DestroyMixer(m_Mixer);
                m_Mixer = nullptr;
            }

            MIX_Quit();
        }

        void RegisterSound(SoundId id, const std::string& filePath)
        {
            if (!m_Mixer) return;
            if (m_AudioMap.count(id)) return;

            MIX_Audio* audio = MIX_LoadAudio(m_Mixer, filePath.c_str(), true);
            if (!audio)
            {
                SDL_Log("MIX_LoadAudio failed for '%s': %s", filePath.c_str(), SDL_GetError());
                return;
            }
            m_AudioMap[id] = audio;
        }

        void PlaySound(SoundId id, int volume)
        {
            if (m_Muted) return;
            if (!m_WorkerThread.joinable()) return;

            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_RequestQueue.push({ id, volume });
            m_Condition.notify_one();
        }

        void SetMuted(bool muted)
        {
            m_Muted = muted;
            if (muted && m_Mixer)
            {
                std::lock_guard<std::mutex> lock(m_QueueMutex);
                while (!m_RequestQueue.empty())
                    m_RequestQueue.pop();
                MIX_StopAllTracks(m_Mixer, 0);
            }
        }

    private:
        void ProcessQueue()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_Condition.wait(lock, [this] {
                    return !m_RequestQueue.empty() || m_Quit;
                    });

                if (m_Quit && m_RequestQueue.empty())
                    break;

                if (!m_RequestQueue.empty())
                {
                    SoundRequest request = m_RequestQueue.front();
                    m_RequestQueue.pop();
                    lock.unlock();

                    auto it = m_AudioMap.find(request.id);
                    if (it != m_AudioMap.end() && it->second && m_Mixer)
                        MIX_PlayAudio(m_Mixer, it->second);
                }
            }
        }

        MIX_Mixer* m_Mixer;
        std::unordered_map<SoundId, MIX_Audio*> m_AudioMap;

        std::queue<SoundRequest> m_RequestQueue;
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        std::thread m_WorkerThread;
        bool m_Quit;
        bool m_Muted;
    };

    SDLSoundSystem::SDLSoundSystem()
        : m_pImpl(std::make_unique<Impl>())
    {
    }

    SDLSoundSystem::~SDLSoundSystem() = default;

    void SDLSoundSystem::RegisterSound(SoundId id, const std::string& filePath)
    {
        m_pImpl->RegisterSound(id, filePath);
    }

    void SDLSoundSystem::PlaySound(SoundId id, int volume)
    {
        m_pImpl->PlaySound(id, volume);
    }

    void SDLSoundSystem::SetMuted(bool muted)
    {
        m_pImpl->SetMuted(muted);
    }
}