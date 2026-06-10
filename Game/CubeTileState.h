#pragma once
#include <memory>

namespace dae
{
    enum class LevelRule
    {
        SingleStep, // level 1: one step reaches target, stays there
        DoubleStep, // level 2: two steps to reach target, stays there
        RevertOnRetouch // level 3: one step to target, second step reverts
    };

    class CubeTileState
    {
    public:
        virtual ~CubeTileState() = default;
        // Returns new state if transition occurs, nullptr if no change
        virtual std::unique_ptr<CubeTileState> OnStep(LevelRule rule) = 0;
        // Row index into the cube spritesheet (0=untouched, 1=intermediate, 2=target)
        virtual int GetRow() const = 0;
        virtual bool IsTarget() const = 0;
    };

    class UntouchedState final : public CubeTileState
    {
    public:
        std::unique_ptr<CubeTileState> OnStep(LevelRule rule) override;
        int GetRow() const override { return 0; }
        bool IsTarget() const override { return false; }
    };

    class IntermediateState final : public CubeTileState
    {
    public:
        std::unique_ptr<CubeTileState> OnStep(LevelRule rule) override;
        int GetRow() const override { return 1; }
        bool IsTarget() const override { return false; }
    };

    class TargetState final : public CubeTileState
    {
    public:
        std::unique_ptr<CubeTileState> OnStep(LevelRule rule) override;
        int GetRow() const override { return 2; }
        bool IsTarget() const override { return true; }
    };
}
