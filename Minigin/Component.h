#pragma once

namespace dae
{
	class GameObject;

	class Component
	{
	public:
		explicit Component(GameObject* pOwner);
		virtual ~Component() = default;

		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		Component& operator=(const Component& other) = delete;
		Component& operator=(Component&& other) = delete;

		virtual void Update() = 0;
		virtual void Render() const {}

		GameObject* GetOwner() const { return m_pOwner; }
		bool IsMarkedForRemoval() const { return m_markedForRemoval; }
		void MarkForRemoval() { m_markedForRemoval = true; }

	protected:
		GameObject* m_pOwner;
		bool m_markedForRemoval{ false };
	};
}