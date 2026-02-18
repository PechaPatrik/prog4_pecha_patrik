#include <SDL3/SDL.h>
#include <format>
#include "FPSComponent.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TextComponent.h"

using namespace dae;

FPSComponent::FPSComponent(GameObject* pOwner)
	: Component(pOwner)
{
}

FPSComponent::~FPSComponent() = default;

void FPSComponent::Update(float deltaTime)
{
	m_frameCount++;
	m_accumulatedTime += deltaTime;

	if (m_accumulatedTime >= m_updateInterval)
	{
		m_currentFPS = static_cast<float>(m_frameCount) / m_accumulatedTime;
		m_frameCount = 0;
		m_accumulatedTime = 0.0f;

		auto textComponent = GetOwner()->GetComponent<TextComponent>();
		if (textComponent)
		{
			textComponent->SetText(std::format("{:.1f} FPS", m_currentFPS));
		}
	}
}