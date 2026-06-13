#pragma once
#include "Command.h"
#include <functional>

namespace dae
{
    class CallbackCommand final : public Command
    {
    public:
        explicit CallbackCommand(std::function<void()> callback)
            : m_callback(std::move(callback))
        {
        }

        void Execute() override
        {
            if (m_callback)
                m_callback();
        }

    private:
        std::function<void()> m_callback;
    };
}
