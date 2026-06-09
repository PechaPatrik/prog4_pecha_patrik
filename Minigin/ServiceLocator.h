#pragma once
#include <memory>
#include "ISoundSystem.h"

namespace dae
{
    class ServiceLocator
    {
    public:
        static ISoundSystem& GetSoundSystem()
        {
            return *m_SoundSystem;
        }

        static void RegisterSoundSystem(std::unique_ptr<ISoundSystem>&& system)
        {
            if (!system)
                m_SoundSystem = std::make_unique<NullSoundSystem>();
            else
                m_SoundSystem = std::move(system);
        }

    private:
        static std::unique_ptr<ISoundSystem> m_SoundSystem;
    };
}