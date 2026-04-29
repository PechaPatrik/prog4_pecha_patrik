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
#include "PlayerComponent.h"
#include "LivesDisplayComponent.h"
#include "ScoreDisplayComponent.h"
#include "PlayerCommands.h"
#include "Scene.h"

#include "SteamObserver.h"

#ifdef USE_STEAMWORKS
#include <steam_api.h>
#endif

#include <filesystem>
namespace fs = std::filesystem;

struct PlayerSetup
{
	dae::PlayerComponent* player;
	dae::MoveComponent* move;
};

static PlayerSetup MakePlayer(dae::Scene& scene,
	const char* sprite, float x, float y, float speed)
{
	auto go = std::make_unique<dae::GameObject>();
	go->SetLocalPosition(x, y);
	go->AddComponent<dae::ImageComponent>(sprite);
	auto* move = go->AddComponent<dae::MoveComponent>(speed);
	auto* player = go->AddComponent<dae::PlayerComponent>(3);
	scene.Add(std::move(go));
	return { player, move };
}

static void MakeLivesDisplay(dae::Scene& scene,
	dae::PlayerComponent* player, int playerIndex, float x, float y,
	std::shared_ptr<dae::Font> font)
{
	auto go = std::make_unique<dae::GameObject>();
	auto* tc = go->AddComponent<dae::TextComponent>(
		"P" + std::to_string(playerIndex + 1) + " Lives: 3", font);
	tc->SetColor({ 255, 255, 255, 255 });
	tc->SetPosition(x, y);
	auto* display = go->AddComponent<dae::LivesDisplayComponent>(playerIndex, 3);
	player->AddObserver(display);
	scene.Add(std::move(go));
}

static void MakeScoreDisplay(dae::Scene& scene,
	dae::PlayerComponent* player, int playerIndex, float x, float y,
	std::shared_ptr<dae::Font> font)
{
	auto go = std::make_unique<dae::GameObject>();
	auto* tc = go->AddComponent<dae::TextComponent>(
		"P" + std::to_string(playerIndex + 1) + " Score: 0", font);
	tc->SetColor({ 255, 255, 255, 255 });
	tc->SetPosition(x, y);
	auto* display = go->AddComponent<dae::ScoreDisplayComponent>(playerIndex);
	player->AddObserver(display);
	scene.Add(std::move(go));
}

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();
	auto& input = dae::InputManager::GetInstance();

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
	auto cc1 = controlsGo->AddComponent<dae::TextComponent>("Use WASD to move Ugg, C to inflict damage, Z and X to get score", font);
	cc1->SetColor({ 255, 255, 255, 255 });
	cc1->SetPosition(10.f, 100.f);
	auto cc2 = controlsGo->AddComponent<dae::TextComponent>("Use D-Pad to move Wrong-way, X to inflict damage, A and B to get score", font);
	cc2->SetColor({ 255, 255, 255, 255 });
	cc2->SetPosition(10.f, 120.f);
	scene.Add(std::move(controlsGo));

	// Ugg and wrong-way
	auto [ugg, uggMove] = MakePlayer(scene, "ugg.png", 300.f, 300.f, 100.f);
	MakeLivesDisplay(scene, ugg, 0, 10.f, 155.f, font);
	MakeScoreDisplay(scene, ugg, 0, 10.f, 175.f, font);

	auto [wrong, wrongMove] = MakePlayer(scene, "wrongway.png", 400.f, 300.f, 200.f);
	MakeLivesDisplay(scene, wrong, 1, 10.f, 200.f, font);
	MakeScoreDisplay(scene, wrong, 1, 10.f, 220.f, font);

	static dae::SteamAchievementObserver steamObs;
	ugg->AddObserver(&steamObs);
	wrong->AddObserver(&steamObs);

	// Keybinds
	input.BindKeyboardCommand(SDL_SCANCODE_W, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ 0.f, -1.f }));
	input.BindKeyboardCommand(SDL_SCANCODE_S, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ 0.f, 1.f }));
	input.BindKeyboardCommand(SDL_SCANCODE_A, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ -1.f, 0.f }));
	input.BindKeyboardCommand(SDL_SCANCODE_D, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(uggMove, glm::vec2{ 1.f, 0.f }));

	input.BindKeyboardCommand(SDL_SCANCODE_C, dae::Controller::KeyState::Down,
		std::make_unique<dae::LoseLifeCommand>(wrong));
	input.BindKeyboardCommand(SDL_SCANCODE_Z, dae::Controller::KeyState::Down,
		std::make_unique<dae::AddScoreCommand>(ugg, 10));
	input.BindKeyboardCommand(SDL_SCANCODE_X, dae::Controller::KeyState::Down,
		std::make_unique<dae::AddScoreCommand>(ugg, 100));

	input.BindControllerCommand(0, dae::Controller::Button::DPadUp, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ 0.f, -1.f }));
	input.BindControllerCommand(0, dae::Controller::Button::DPadDown, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ 0.f, 1.f }));
	input.BindControllerCommand(0, dae::Controller::Button::DPadLeft, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ -1.f, 0.f }));
	input.BindControllerCommand(0, dae::Controller::Button::DPadRight, dae::Controller::KeyState::Pressed,
		std::make_unique<dae::MoveCommand>(wrongMove, glm::vec2{ 1.f, 0.f }));

	input.BindControllerCommand(0, dae::Controller::Button::X, dae::Controller::KeyState::Down,
		std::make_unique<dae::LoseLifeCommand>(ugg));
	input.BindControllerCommand(0, dae::Controller::Button::A, dae::Controller::KeyState::Down,
		std::make_unique<dae::AddScoreCommand>(wrong, 10));
	input.BindControllerCommand(0, dae::Controller::Button::B, dae::Controller::KeyState::Down,
		std::make_unique<dae::AddScoreCommand>(wrong, 100));
}

int main(int, char*[]) {
#ifdef USE_STEAMWORKS
	SteamAPI_Init();
#endif
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
#ifdef USE_STEAMWORKS
	SteamAPI_Shutdown();
#endif
	return 0;
}
