#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"

using namespace dae;

GameObject::~GameObject() = default;

void GameObject::Update()
{
	RemoveMarkedComponents();

	for (auto& component : m_components)
	{
		if (!component->IsMarkedForRemoval())
		{
			component->Update();
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

void GameObject::RemoveMarkedComponents()
{
	if (m_componentsToRemove.empty())
		return;

	// Remove from vector
	auto it = std::remove_if(m_components.begin(), m_components.end(),
		[this](const std::unique_ptr<Component>& component) {
			return std::find(m_componentsToRemove.begin(), m_componentsToRemove.end(),
				component.get()) != m_componentsToRemove.end();
		});

	m_components.erase(it, m_components.end());
	m_componentsToRemove.clear();
}
