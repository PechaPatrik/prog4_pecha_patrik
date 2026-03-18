#pragma once

namespace dae
{
    enum class GameEvent
    {
        PlayerDied,
        ScoreChanged,
        LivesChanged,
    };

    class IObserver
    {
    public:
        virtual ~IObserver() = default;
        virtual void OnNotify(GameEvent event, int value) = 0;
    };
}