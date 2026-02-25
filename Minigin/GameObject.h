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
		~GameObject();

		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		void Update(float deltaTime);
		void FixedUpdate(float fixedTimeStep);
		void Render() const;

		void SetLocalPosition(float x, float y);
		void SetLocalPosition(const glm::vec3& pos);
		const glm::vec3& GetWorldPosition();

		void SetParent(GameObject* pParent, bool keepWorldPosition = false);
		GameObject* GetParent() const { return m_pParent; }

		size_t GetChildCount() const { return m_children.size(); }
		GameObject* GetChildAt(int index) const { return m_children[index]; }

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
		void UpdateWorldPosition();
		bool IsChild(GameObject* parent) const;
		void AddChild(GameObject* child);
		void RemoveChild(GameObject* child);
		void SetPositionDirty();

		glm::vec3 m_localPosition{};
		mutable glm::vec3 m_worldPosition{};
		mutable bool m_positionIsDirty{ true };

		GameObject* m_pParent{ nullptr };
		std::vector<GameObject*> m_children{};

		std::vector<std::unique_ptr<Component>> m_components{};
		bool m_markedForRemoval{ false };
	};
}
