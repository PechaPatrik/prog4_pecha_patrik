#pragma once
#include "ISoundSystem.h"
#include <memory>

namespace dae
{
    class SDLSoundSystem final : public ISoundSystem
    {
    public:
        SDLSoundSystem();
        ~SDLSoundSystem() override;

        SDLSoundSystem(const SDLSoundSystem&) = delete;
        SDLSoundSystem(SDLSoundSystem&&) = delete;
        SDLSoundSystem& operator=(const SDLSoundSystem&) = delete;
        SDLSoundSystem& operator=(SDLSoundSystem&&) = delete;

        void RegisterSound(SoundId id, const std::string& filePath) override;
        void PlaySound(SoundId id, int volume = 128) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}