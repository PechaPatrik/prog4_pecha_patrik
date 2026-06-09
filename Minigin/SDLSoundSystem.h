#pragma once
#include "ISoundSystem.h"
#include <memory>
#include <string>

namespace dae
{
    class SDLSoundSystem final : public ISoundSystem
    {
    public:
        explicit SDLSoundSystem(const std::string& dataPath);
        ~SDLSoundSystem() override;

        SDLSoundSystem(const SDLSoundSystem&) = delete;
        SDLSoundSystem(SDLSoundSystem&&) = delete;
        SDLSoundSystem& operator=(const SDLSoundSystem&) = delete;
        SDLSoundSystem& operator=(SDLSoundSystem&&) = delete;

        void PlaySound(SoundId id, int volume = 128) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}