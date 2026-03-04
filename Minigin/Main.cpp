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
#include "ImageComponent.h"
#include "RotatorComponent.h"
#include "Scene.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	//Background
	auto bgGo = std::make_unique<dae::GameObject>();
	bgGo->AddComponent<dae::ImageComponent>("background.png");
	scene.Add(std::move(bgGo));

	//Logo
	auto loGo = std::make_unique<dae::GameObject>();
	loGo->AddComponent<dae::ImageComponent>("logo.png");
	loGo->SetLocalPosition(358.f, 180.f);
	scene.Add(std::move(loGo));

	//Title text
	auto titleGo = std::make_unique<dae::GameObject>();
	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto tc = titleGo->AddComponent<dae::TextComponent>("Programming 4 Assignment", font);
	tc->SetColor({ 255, 255, 0, 255 });
	tc->SetPosition(292.f, 20.f);
	scene.Add(std::move(titleGo));

	//FPS counter
	auto fpsGo = std::make_unique<dae::GameObject>();
	auto fpsText = fpsGo->AddComponent<dae::TextComponent>("0 FPS", font);
	fpsText->SetColor({ 255, 255, 255, 255 });
	fpsText->SetPosition(10.f, 10.f);
	fpsGo->AddComponent<dae::FPSComponent>();
	scene.Add(std::move(fpsGo));

	//Position GO around which Ugg rotates
	auto posGo = std::make_unique<dae::GameObject>();
	posGo->SetLocalPosition(300.f, 300.f);

	//Ugg
	auto ugGo = std::make_unique<dae::GameObject>();
	ugGo->SetParent(posGo.get());
	ugGo->AddComponent<dae::ImageComponent>("ugg.png");
	ugGo->AddComponent<dae::RotatorComponent>(10.f, -10.f);

	//Wrong-way
	auto wronGo = std::make_unique<dae::GameObject>();
	wronGo->SetParent(ugGo.get());
	wronGo->AddComponent<dae::ImageComponent>("wrongway.png");
	wronGo->AddComponent<dae::RotatorComponent>(40.f, 7.5f);
	scene.Add(std::move(posGo));
	scene.Add(std::move(ugGo));
	scene.Add(std::move(wronGo));
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
