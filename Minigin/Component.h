#pragma once

namespace dae
{
	class GameObject;

	class Component
	{
	public:
		virtual ~Component() = default;

		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		Component& operator=(const Component& other) = delete;
		Component& operator=(Component&& other) = delete;

		virtual void Update(float deltaTime) = 0;
		virtual void FixedUpdate(float) {}
		virtual void Render() const {}

		bool IsMarkedForRemoval() const { return m_markedForRemoval; }
		void MarkForRemoval() { m_markedForRemoval = true; }

	private:
		const GameObject* m_pOwner;
	protected:
		explicit Component(GameObject* pOwner);
		const GameObject* GetOwner() const { return m_pOwner; }
		bool m_markedForRemoval{ false };
	};
}