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

        void RegisterSound(SoundId id, const std::string& filePath) override
        {
            m_pReal->RegisterSound(id, filePath);
            std::cout << "RegisterSound id=" << static_cast<int>(id) << " path=" << filePath << "\n";
        }

        void PlaySound(SoundId id, int volume) override
        {
            m_pReal->PlaySound(id, volume);
            std::cout << "PlaySound id=" << static_cast<int>(id) << " volume=" << volume << "\n";
        }

        void SetMuted(bool muted) override
        {
            m_pReal->SetMuted(muted);
            std::cout << "SetMuted " << (muted ? "true" : "false") << "\n";
        }

    private:
        std::unique_ptr<ISoundSystem> m_pReal;
    };
}