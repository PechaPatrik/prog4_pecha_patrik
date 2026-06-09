#pragma once
#include "ISoundSystem.h"
#include <memory>
#include <iostream>

namespace dae
{
    class LoggingSoundSystem final : public ISoundSystem
    {
    public:
        explicit LoggingSoundSystem(std::unique_ptr<ISoundSystem>&& real)
            : m_pReal(std::move(real))
        {
        }

        void PlaySound(SoundId id, int volume) override
        {
            m_pReal->PlaySound(id, volume);
            std::cout << "PlaySound id=" << static_cast<int>(id)
                << " volume=" << volume << "\n";
        }

    private:
        std::unique_ptr<ISoundSystem> m_pReal;
    };
}