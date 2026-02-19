#include <stdexcept>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>

#if WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#endif

#include <SDL3/SDL.h>
//#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Minigin.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Renderer.h"
#include "ResourceManager.h"

SDL_Window* g_window{};

void LogSDLVersion(const std::string& message, int major, int minor, int patch)
{
#if WIN32
	std::stringstream ss;
	ss << message << major << "." << minor << "." << patch << "\n";
	OutputDebugString(ss.str().c_str());
#else
	std::cout << message << major << "." << minor << "." << patch << "\n";
#endif
}

#ifdef __EMSCRIPTEN__
#include "emscripten.h"

void LoopCallback(void* arg)
{
	static_cast<dae::Minigin*>(arg)->RunOneFrame();
}
#endif

// Why bother with this? Because sometimes students have a different SDL version installed on their pc.
// That is not a problem unless for some reason the dll's from this project are not copied next to the exe.
// These entries in the debug output help to identify that issue.
void PrintSDLVersion()
{
	LogSDLVersion("Compiled with SDL", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
	int version = SDL_GetVersion();
	LogSDLVersion("Linked with SDL ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
	// LogSDLVersion("Compiled with SDL_image ",SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_MICRO_VERSION);
	// version = IMG_Version();
	// LogSDLVersion("Linked with SDL_image ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
	LogSDLVersion("Compiled with SDL_ttf ",	SDL_TTF_MAJOR_VERSION, SDL_TTF_MINOR_VERSION,SDL_TTF_MICRO_VERSION);
	version = TTF_Version();
	LogSDLVersion("Linked with SDL_ttf ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version),	SDL_VERSIONNUM_MICRO(version));
}

namespace dae {
	Minigin::Minigin(const std::filesystem::path& dataPath)
	{
		PrintSDLVersion();

		if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
		{
			SDL_Log("Renderer error: %s", SDL_GetError());
			throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
		}

		g_window = SDL_CreateWindow(
			"Programming 4 assignment",
			1024,
			576,
			SDL_WINDOW_OPENGL
		);
		if (g_window == nullptr)
		{
			throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());
		}

		Renderer::GetInstance().Init(g_window);
		ResourceManager::GetInstance().Init(dataPath);
	}

	Minigin::~Minigin()
	{
		Renderer::GetInstance().Destroy();
		SDL_DestroyWindow(g_window);
		g_window = nullptr;
		SDL_Quit();
	}

	void dae::Minigin::Run(const std::function<void()>& load)
	{
		load();

#ifndef __EMSCRIPTEN__
		while (!m_quit)
			RunOneFrame();
#else
		emscripten_set_main_loop_arg(&LoopCallback, this, 0, true);
#endif
	}

	void Minigin::RunOneFrame()
	{
		m_quit = !InputManager::GetInstance().ProcessInput();

		using clock = std::chrono::high_resolution_clock;
		using duration = std::chrono::duration<float>;

		static auto frameStart = clock::now();
		static float accumulatedTime = 0.f;

		auto now = clock::now();
		float deltaTime = duration(now - frameStart).count();
		frameStart = now;
		const float maxDeltaTime = 0.1f; // 10FPS minimum simulation

		if (m_maxFPS <= 0) //unlimited FPS
		{
			if (deltaTime > maxDeltaTime) deltaTime = maxDeltaTime;
			SceneManager::GetInstance().Update(static_cast<float>(deltaTime));
		}
		else
		{
			const float fixedStep = 1.f / m_maxFPS;

			if (deltaTime > maxDeltaTime) deltaTime = maxDeltaTime;
			accumulatedTime += deltaTime;

			while (accumulatedTime >= fixedStep)
			{
				SceneManager::GetInstance().Update(static_cast<float>(fixedStep));
				accumulatedTime -= fixedStep;
			}

			std::this_thread::sleep_until(frameStart + duration{ fixedStep });
		}

		Renderer::GetInstance().Render();
	}
}
