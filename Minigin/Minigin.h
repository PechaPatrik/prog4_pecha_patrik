#pragma once
#include <chrono>
#include <string>
#include <functional>
#include <filesystem>

namespace dae
{
	class Minigin final
	{
	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);
		void RunOneFrame();

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	private:
		bool m_quit{};
		const float m_maxDeltaTime{ 0.1f };
		float m_fixedTimeStep{ 1.f / 60.f };
		float m_accumulatedTime = 0.f;
		std::chrono::high_resolution_clock::time_point m_lastTime{ std::chrono::high_resolution_clock::now() };
	};
}