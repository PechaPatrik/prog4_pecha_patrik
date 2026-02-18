#include "SceneManager.h"
#include "Scene.h"

namespace dae {
	void SceneManager::Update()
	{
		for (auto& scene : m_scenes)
		{
			scene->Update();
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
}

