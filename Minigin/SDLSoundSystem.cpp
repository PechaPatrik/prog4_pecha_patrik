#include "SDLSoundSystem.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

namespace dae
{
    struct SoundRequest
    {
        SoundId id;
        int volume;
    };

    struct SDLSoundSystem::Impl
    {
        Impl(const std::string& dataPath)
            : m_Mixer(nullptr)
            , m_DamageAudio(nullptr)
            , m_Quit(false)
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

            fs::path audioPath = fs::path(dataPath) / "Audio" / "damage.mp3";
            m_DamageAudio = MIX_LoadAudio(m_Mixer, audioPath.string().c_str(), true);
            if (!m_DamageAudio)
            {
                SDL_Log("MIX_LoadAudio failed for '%s': %s",
                    audioPath.string().c_str(), SDL_GetError());
                MIX_DestroyMixer(m_Mixer);
                m_Mixer = nullptr;
                MIX_Quit();
                return;
            }

            m_WorkerThread = std::thread(&Impl::ProcessQueue, this);
            SDL_Log("SDLSoundSystem initialized successfully");
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

            if (m_DamageAudio)
            {
                MIX_DestroyAudio(m_DamageAudio);
                m_DamageAudio = nullptr;
            }

            if (m_Mixer)
            {
                MIX_DestroyMixer(m_Mixer);
                m_Mixer = nullptr;
            }

            MIX_Quit();
        }

        void PlaySound(SoundId id, int volume)
        {
            if (!m_WorkerThread.joinable())
                return;

            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_RequestQueue.push({ id, volume });
            m_Condition.notify_one();
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

                    if (request.id == SoundId::PlayerDamage && m_DamageAudio && m_Mixer)
                        MIX_PlayAudio(m_Mixer, m_DamageAudio);
                }
            }
        }

        MIX_Mixer* m_Mixer;
        MIX_Audio* m_DamageAudio;

        std::queue<SoundRequest> m_RequestQueue;
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        std::thread m_WorkerThread;
        bool m_Quit;
    };

    SDLSoundSystem::SDLSoundSystem(const std::string& dataPath)
        : m_pImpl(std::make_unique<Impl>(dataPath))
    {
    }

    SDLSoundSystem::~SDLSoundSystem() = default;

    void SDLSoundSystem::PlaySound(SoundId id, int volume)
    {
        m_pImpl->PlaySound(id, volume);
    }
}