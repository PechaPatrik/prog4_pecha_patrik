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

	if (m_texture)
	{
		const auto& pos = m_transform.GetPosition();
		Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
	}
}

void GameObject::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void GameObject::SetPosition(float x, float y)
{
	m_transform.SetPosition(x, y, 0.0f);
}
