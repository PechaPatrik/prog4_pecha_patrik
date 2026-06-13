#pragma once
#include "SoundId.h"
#include <string>

namespace dae
{
    class ISoundSystem
    {
    public:
        virtual ~ISoundSystem() = default;
        virtual void RegisterSound(SoundId id, const std::string& filePath) = 0;
        virtual void PlaySound(SoundId id, int volume = 80) = 0;
        virtual void SetMuted(bool muted) = 0;
    };

    class NullSoundSystem final : public ISoundSystem
    {
    public:
        void RegisterSound(SoundId, const std::string&) override {}
        void PlaySound(SoundId, int) override {}
        void SetMuted(bool) override {}
    };
}