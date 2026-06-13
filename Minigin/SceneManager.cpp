#include "SceneManager.h"
#include "Scene.h"

namespace dae {
    void SceneManager::Update(float deltaTime)
    {
        for (auto& scene : m_scenes)
        {
            if (!scene->IsMarkedForRemoval() && !scene->IsPaused())
            {
                scene->Update(deltaTime);
            }
        }

        std::erase_if(m_scenes, [](const auto& scene) {
            return scene->IsMarkedForRemoval();
            });
    }

    void dae::SceneManager::FixedUpdate(float fixedTimeStep)
    {
        for (auto& scene : m_scenes)
        {
            if (!scene->IsMarkedForRemoval() && !scene->IsPaused())
            {
                scene->FixedUpdate(fixedTimeStep);
            }
        }
    }

    void SceneManager::Render()
    {
        for (const auto& scene : m_scenes)
        {
            scene->Render();
        }
    }

    Scene& SceneManager::CreateScene()
    {
        m_scenes.emplace_back(new Scene());
        return *m_scenes.back();
    }

    void SceneManager::MarkAllScenesForRemoval()
    {
        for (auto& scene : m_scenes)
            scene->MarkForRemoval();
    }
}
