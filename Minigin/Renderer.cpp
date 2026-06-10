#include <stdexcept>
#include <cstring>
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include "Renderer.h"
#include "SceneManager.h"
#include "Texture2D.h"

void dae::Renderer::Init(SDL_Window* window)
{
	m_window = window;
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	m_renderer = SDL_CreateRenderer(window, nullptr);
	if (m_renderer == nullptr)
	{
		std::cout << "Failed to create the renderer: " << SDL_GetError() << "\n";
		throw std::runtime_error(std::string("SDL_CreateRenderer Error: ") + SDL_GetError());
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
#if __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename = NULL;
#endif

	ImGui_ImplSDL3_InitForSDLRenderer(window, m_renderer);
	ImGui_ImplSDLRenderer3_Init(m_renderer);
}

void dae::Renderer::Render() const
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	const auto& color = GetBackgroundColor();
	SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(m_renderer);

	SceneManager::GetInstance().Render();

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
	SDL_RenderPresent(m_renderer);
}

void dae::Renderer::Destroy()
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	if (m_renderer != nullptr)
	{
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}
}

void dae::Renderer::RenderTexture(const Texture2D& texture, const float x, const float y) const
{
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	SDL_GetTextureSize(texture.GetSDLTexture(), &dst.w, &dst.h);
	SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), nullptr, &dst);
}

void dae::Renderer::RenderTexture(const Texture2D& texture, const float x, const float y, const float width, const float height) const
{
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;
	SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), nullptr, &dst);
}

SDL_Renderer* dae::Renderer::GetSDLRenderer() const { return m_renderer; }

void dae::Renderer::RenderTextureSrc(const Texture2D& texture, float x, float y, int srcX, int srcY, int srcW, int srcH) const
{
	SDL_FRect src{};
	src.x = static_cast<float>(srcX);
	src.y = static_cast<float>(srcY);
	src.w = static_cast<float>(srcW);
	src.h = static_cast<float>(srcH);
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	dst.w = static_cast<float>(srcW);
	dst.h = static_cast<float>(srcH);
	SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), &src, &dst);
}

void dae::Renderer::RenderTextureSrcScaled(const Texture2D& texture, float x, float y, int srcX, int srcY, int srcW, int srcH, float scale) const
{
	// 0.5px inset on source rect prevents texture bleeding
	static constexpr float INSET = 0.5f;
	SDL_FRect src{};
	src.x = static_cast<float>(srcX) + INSET;
	src.y = static_cast<float>(srcY) + INSET;
	src.w = static_cast<float>(srcW) - INSET * 2.f;
	src.h = static_cast<float>(srcH) - INSET * 2.f;
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	dst.w = static_cast<float>(srcW) * scale;
	dst.h = static_cast<float>(srcH) * scale;
	SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), &src, &dst);
}