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
#include "TTCComponent.h"
#include "InputManager.h"
#include "MoveComponent.h"
#include "MoveCommand.h"
#include "Controller.h"
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

	//Controls
	auto controlsGo = std::make_unique<dae::GameObject>();
	font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
	auto cc1 = controlsGo->AddComponent<dae::TextComponent>("Use WASD to move Ugg", font);
	cc1->SetColor({ 255, 255, 255, 255 });
	cc1->SetPosition(10.f, 100.f);
	auto cc2 = controlsGo->AddComponent<dae::TextComponent>("Use D-Pad to move Wrong-way", font);
	cc2->SetColor({ 255, 255, 255, 255 });
	cc2->SetPosition(10.f, 120.f);
	scene.Add(std::move(controlsGo));

	//Ugg
	auto uggGo = std::make_unique<dae::GameObject>();
	uggGo->SetLocalPosition(300.f, 300.f);
	uggGo->AddComponent<dae::ImageComponent>("ugg.png");
	auto* uggMove = uggGo->AddComponent<dae::MoveComponent>(100.f);

	auto& input = dae::InputManager::GetInstance();

	input.BindKeyboardCommand(SDL_SCANCODE_W, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ 0.f, -1.f }));
	input.BindKeyboardCommand(SDL_SCANCODE_S, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ 0.f,  1.f }));
	input.BindKeyboardCommand(SDL_SCANCODE_A, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ -1.f, 0.f }));
	input.BindKeyboardCommand(SDL_SCANCODE_D, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ 1.f, 0.f }));

	scene.Add(std::move(uggGo));


	//Wrong-way
	auto wrongGo = std::make_unique<dae::GameObject>();
	wrongGo->SetLocalPosition(400.f, 300.f);
	wrongGo->AddComponent<dae::ImageComponent>("wrongway.png");
	auto* wrongMove = wrongGo->AddComponent<dae::MoveComponent>(200.f);

	input.BindControllerCommand(0, dae::Controller::Button::DPadUp, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ 0.f, -1.f }));
	input.BindControllerCommand(0, dae::Controller::Button::DPadDown, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ 0.f,  1.f }));
	input.BindControllerCommand(0, dae::Controller::Button::DPadLeft, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ -1.f, 0.f }));
	input.BindControllerCommand(0, dae::Controller::Button::DPadRight, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ 1.f, 0.f }));

	scene.Add(std::move(wrongGo));
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
