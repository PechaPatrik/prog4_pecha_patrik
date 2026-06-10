#include "CubeTileState.h"

namespace dae
{
    std::unique_ptr<CubeTileState> UntouchedState::OnStep(LevelRule rule)
    {
        if (rule == LevelRule::DoubleStep)
            return std::make_unique<IntermediateState>();
        return std::make_unique<TargetState>();
    }

    std::unique_ptr<CubeTileState> IntermediateState::OnStep(LevelRule /*rule*/)
    {
        return std::make_unique<TargetState>();
    }

    std::unique_ptr<CubeTileState> TargetState::OnStep(LevelRule rule)
    {
        if (rule == LevelRule::RevertOnRetouch)
            return std::make_unique<UntouchedState>();
        return nullptr;
    }
}
