#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"

using namespace dae;

void GameObject::Update(float deltaTime)
{
	for (auto& component : m_components)
	{
		if (!component->IsMarkedForRemoval())
		{
			component->Update(deltaTime);
		}
	}

	std::erase_if(m_components, [](const auto& component) {
		return component->IsMarkedForRemoval();
		});
}

void dae::GameObject::FixedUpdate(float fixedTimeStep)
{
	for (auto& component : m_components)
	{
		if (!component->IsMarkedForRemoval())
		{
			component->FixedUpdate(fixedTimeStep);
		}
	}
}

void GameObject::Render() const
{
	for (const auto& component : m_components)
	{
		if (!component->IsMarkedForRemoval())
		{
			component->Render();
		}
	}
}

void GameObject::SetPosition(float x, float y)
{
	m_transform.SetPosition(x, y, 0.0f);
}
