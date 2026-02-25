#pragma once
#include <string>
#include <memory>
#include "Transform.h"
#include "Component.h"

namespace dae
{
	class Texture2D;
	class GameObject final
	{
	public:
		GameObject() = default;
		~GameObject() = default;

		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		void Update(float deltaTime);
		void FixedUpdate(float fixedTimeStep);
		void Render() const;

		void SetPosition(float x, float y);
		const glm::vec3& GetPosition() const { return m_transform.GetPosition(); }

		template<typename T, typename... Args>
		T* AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

			auto pComponent = std::make_unique<T>(this, std::forward<Args>(args)...);
			T* pRawComponent = pComponent.get();
			m_components.push_back(std::move(pComponent));
			return pRawComponent;
		}

		template<typename T>
		T* GetComponent() const
		{
			for (const auto& component : m_components)
			{
				T* result = dynamic_cast<T*>(component.get());
				if (result)
					return result;
			}
			return nullptr;
		}

		template<typename T>
		bool HasComponent() const
		{
			for (const auto& component : m_components)
			{
				if (dynamic_cast<T*>(component.get()))
					return true;
			}
			return false;
		}

		template<typename T>
		void RemoveComponent()
		{
			for (const auto& component : m_components)
			{
				if (dynamic_cast<T*>(component.get()))
				{
					component->MarkForRemoval();
					return;
				}
			}
		}

		void MarkForRemoval() { m_markedForRemoval = true; }
		bool IsMarkedForRemoval() const { return m_markedForRemoval; }
	private:
		Transform m_transform{};
		std::vector<std::unique_ptr<Component>> m_components{};
		bool m_markedForRemoval{ false };
	};
}
