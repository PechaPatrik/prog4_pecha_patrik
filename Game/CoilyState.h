#pragma once
#include <memory>

namespace dae
{
    class CoilyComponent;

    class CoilyBaseState
    {
    public:
        virtual ~CoilyBaseState() = default;
        virtual std::unique_ptr<CoilyBaseState> Update(float deltaTime, CoilyComponent& coily) = 0;
        virtual int GetCol(bool inAir) const = 0;
        virtual bool IsEgg() const = 0;
    };

    class CoilyEggState final : public CoilyBaseState
    {
    public:
        explicit CoilyEggState(float hopInterval = 0.5f)
            : m_hopInterval(hopInterval) {
        }
        std::unique_ptr<CoilyBaseState> Update(float deltaTime, CoilyComponent& coily) override;
        int GetCol(bool inAir) const override { return inAir ? 1 : 0; }
        bool IsEgg() const override { return true; }
        float GetHopInterval() const { return m_hopInterval; }

    private:
        float m_hopTimer{ 0.f };
        float m_hopInterval;
    };

    class CoilySnakeState final : public CoilyBaseState
    {
    public:
        explicit CoilySnakeState(float hopInterval = 0.4f)
            : m_hopInterval(hopInterval) {
        }
        std::unique_ptr<CoilyBaseState> Update(float deltaTime, CoilyComponent& coily) override;
        int GetCol(bool inAir) const override;
        bool IsEgg() const override { return false; }
        void SetDirection(int dir) { m_direction = dir; }

    private:
        float m_hopTimer{ 0.f };
        float m_hopInterval;
        int m_direction{ 2 };
    };
}