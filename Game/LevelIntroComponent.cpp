#include "LevelIntroComponent.h"
#include "SceneLoader.h"

namespace dae
{
    void LevelIntroComponent::Update(float deltaTime)
    {
        if (m_done) return;

        m_timer += deltaTime;
        if (m_timer >= m_duration)
        {
            m_done = true;
            SceneLoader::GetInstance().LoadGameplay(m_levelIndex, 0);
        }
    }
}
