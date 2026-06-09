#include "ServiceLocator.h"

namespace dae
{
    std::unique_ptr<ISoundSystem> ServiceLocator::m_SoundSystem = std::make_unique<NullSoundSystem>();
}