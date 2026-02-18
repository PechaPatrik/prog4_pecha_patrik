#pragma once
#include <string>
#include <memory>
#include <typeindex>
#include "Transform.h"
#include "Component.h"

namespace dae
{
	class Texture2D;
	class GameObject final
	{
	private:
		Transform m_transform{};
		std::shared_ptr<Texture2D> m_texture{};
		std::vector<std::unique_ptr<Component>> m_components{};
		std::vector<Component*> m_componentsToRemove{};
		bool m_markedForRemoval{ false };
	public:
		GameObject() = default;
		virtual ~GameObject();

		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		virtual void Update();
		virtual void Render() const;

		void SetTexture(const std::string& filename);
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
				if (typeid(*component) == typeid(T))
				{
					return static_cast<T*>(component.get());
				}
			}
			return nullptr;
		}

		template<typename T>
		bool HasComponent() const
		{
			for (const auto& component : m_components)
			{
				if (typeid(*component) == typeid(T))
				{
					return true;
				}
			}
			return false;
		}

		template<typename T>
		void RemoveComponent()
		{
			for (const auto& component : m_components)
			{
				if (typeid(*component) == typeid(T))
				{
					component->MarkForRemoval();
					m_componentsToRemove.push_back(component.get());
				}
			}
		}

		void RemoveMarkedComponents();
		void MarkForRemoval() { m_markedForRemoval = true; }
		bool IsMarkedForRemoval() const { return m_markedForRemoval; }
	};
}
