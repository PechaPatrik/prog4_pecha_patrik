#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Scene.h"
#include "Singleton.h"

namespace dae
{
    class Scene;
    class SceneManager final : public Singleton<SceneManager>
    {
    public:
        Scene& CreateScene();
        void MarkAllScenesForRemoval();

        void Update(float deltaTime);
        void FixedUpdate(float fixedTimeStep);
        void Render();

        // Returns the last non-marked scene, or nullptr if none exists.
        // Used by SceneLoader to pause/unpause the gameplay scene.
        Scene* GetLastScene()
        {
            for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
            {
                if (!m_scenes[i]->IsMarkedForRemoval())
                    return m_scenes[i].get();
            }
            return nullptr;
        }

        // Returns the first non-marked scene (the gameplay scene when an overlay is on top).
        Scene* GetFirstScene()
        {
            for (auto& s : m_scenes)
            {
                if (!s->IsMarkedForRemoval())
                    return s.get();
            }
            return nullptr;
        }

    private:
        friend class Singleton<SceneManager>;
        SceneManager() = default;
        std::vector<std::unique_ptr<Scene>> m_scenes{};
    };
}
