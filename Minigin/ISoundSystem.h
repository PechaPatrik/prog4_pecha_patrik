#pragma once
#include "SoundId.h"

namespace dae
{
    class ISoundSystem
    {
    public:
        virtual ~ISoundSystem() = default;
        virtual void PlaySound(SoundId id, int volume = 128) = 0;
    };

    class NullSoundSystem final : public ISoundSystem
    {
    public:
        void PlaySound(SoundId, int) override {}
    };
}