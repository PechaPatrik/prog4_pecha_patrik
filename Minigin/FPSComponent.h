#pragma once
#include "Component.h"
#include <memory>
#include <string>

namespace dae
{
	class Font;
	class TextComponent;

	class FPSComponent final : public Component
	{
	public:
		FPSComponent(GameObject* pOwner);
		~FPSComponent() override;

		FPSComponent(const FPSComponent& other) = delete;
		FPSComponent(FPSComponent&& other) = delete;
		FPSComponent& operator=(const FPSComponent& other) = delete;
		FPSComponent& operator=(FPSComponent&& other) = delete;

		void Update(float deltaTime) override;

	private:
		float m_accumulatedTime{ 0.0f };
		int m_frameCount{ 0 };
		float m_currentFPS{ 0.0f };
		const float m_updateInterval{ 1.f }; // Update FPS text every second
	};
}