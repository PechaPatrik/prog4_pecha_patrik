#pragma once
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "GameObject.h"

namespace dae
{
	class Scene final
	{
	public:
		~Scene() = default;
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

		void Update(float deltaTime);
		void FixedUpdate(float fixedTimeStep);
		void Render() const;

		void Add(std::unique_ptr<GameObject> object);
		void Remove(const GameObject& object);
		void RemoveAll();
		void MoveToBack(const GameObject* object)
		{
			auto it = std::find_if(m_objects.begin(), m_objects.end(),
				[object](const std::unique_ptr<GameObject>& o) { return o.get() == object; });
			if (it == m_objects.end() || it == m_objects.begin()) return;
			std::rotate(m_objects.begin(), it, it + 1);
		}

		void MarkForRemoval() { m_markedForRemoval = true; }
		bool IsMarkedForRemoval() const { return m_markedForRemoval; }
	private:
		friend class SceneManager;
		explicit Scene() = default;
		std::vector < std::unique_ptr<GameObject>> m_objects{};
		bool m_markedForRemoval{ false };
	};

}