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

void dae::GameObject::SetLocalPosition(const glm::vec3& pos)
{
	m_localPosition = pos;
	SetPositionDirty();
}

void GameObject::SetLocalPosition(float x, float y)
{
	SetLocalPosition({ x, y, 0.f });
}

const glm::vec3& GameObject::GetWorldPosition()
{
	if (m_positionIsDirty)
		UpdateWorldPosition();
	return m_worldPosition;
}

void GameObject::UpdateWorldPosition()
{
	if (m_positionIsDirty)
	{
		if (m_pParent == nullptr)
			m_worldPosition = m_localPosition;
		else
			m_worldPosition = m_pParent->GetWorldPosition() + m_localPosition;
	}
	m_positionIsDirty = false;
}

bool GameObject::IsChild(GameObject* parent) const
{
	for (auto* child : m_children)
	{
		if (child == parent)
			return true;
		if (child->IsChild(parent))
			return true;
	}
	return false;
}

void GameObject::SetParent(GameObject* parent, bool keepWorldPosition)
{
	if (IsChild(parent) || parent == this || m_pParent == parent)
		return;

	if (parent == nullptr)
		SetLocalPosition(GetWorldPosition());
	else
	{
		if (keepWorldPosition)
		{
			SetLocalPosition(GetWorldPosition() - parent->GetWorldPosition());
		}
		SetPositionDirty();
	}

	if (m_pParent) m_pParent->RemoveChild(this);
	m_pParent = parent;
	if (m_pParent) m_pParent->AddChild(this);
}

void GameObject::AddChild(GameObject* child)
{
	m_children.push_back(child);
}

void GameObject::RemoveChild(GameObject* child)
{
	m_children.erase(
		std::remove(m_children.begin(), m_children.end(), child),
		m_children.end()
	);
}

void GameObject::SetPositionDirty()
{
	m_positionIsDirty = true;
	for (auto* child : m_children)
	{
		child->SetPositionDirty();
	}
}