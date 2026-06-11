#pragma once
#include "Component.h"
#include "GameStateManager.h"
#include "Scene.h"

namespace dae
{
    class GsmUpdaterComponent final : public Component
    {
    public:
        GsmUpdaterComponent(GameObject* pOwner, Scene* scene)
            : Component(pOwner)
            , m_scene(scene)
        {
        }

        void Update(float dt) override
        {
            GameStateManager::GetInstance().Update(dt, m_scene);
        }

    private:
        Scene* m_scene;
    };
}