#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "Scene.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	//Background
	auto go = std::make_unique<dae::GameObject>();
	go->SetTexture("background.png");
	scene.Add(std::move(go));

	//Logo
	go = std::make_unique<dae::GameObject>();
	go->SetTexture("logo.png");
	go->SetPosition(358, 180);
	scene.Add(std::move(go));

	//Title text
	auto titleGo = std::make_unique<dae::GameObject>();
	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto tc = titleGo->AddComponent<dae::TextComponent>("Programming 4 Assignment", font);
	tc->SetColor({ 255, 255, 0, 255 });
	tc->SetPosition(292, 20);
	scene.Add(std::move(titleGo));

	//FPS counter
	auto fpsGo = std::make_unique<dae::GameObject>();
	auto fpsText = fpsGo->AddComponent<dae::TextComponent>("0 FPS", font);
	fpsText->SetColor({ 255, 255, 255, 255 });
	fpsText->SetPosition(10, 10);
	fpsGo->AddComponent<dae::FPSComponent>();
	scene.Add(std::move(fpsGo));
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
    return 0;
}
