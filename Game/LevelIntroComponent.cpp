#include "LevelIntroComponent.h"
#include "SceneLoader.h"
#include "ServiceLocator.h"
#include "SoundId.h"

namespace dae
{
    void LevelIntroComponent::Update(float deltaTime)
    {
        if (m_done) return;

        if (!m_soundPlayed)
        {
            m_soundPlayed = true;
            ServiceLocator::GetSoundSystem().PlaySound(SoundId::LevelScreenTune);
        }

        m_timer += deltaTime;
        if (m_timer >= m_duration)
        {
            m_done = true;
            SceneLoader::GetInstance().LoadGameplay(m_levelIndex, 0);
        }
    }
}