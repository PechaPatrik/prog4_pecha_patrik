#pragma once
#include "Singleton.h"
#include "Command.h"
#include "Controller.h"

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <map>
#include <tuple>

namespace dae
{
	class InputManager final : public Singleton<InputManager>
	{
	public:
		using KeyState = Controller::KeyState;

		void BindKeyboardCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command);
		void UnbindKeyboardCommand(SDL_Scancode key, KeyState state);

		void BindControllerCommand(unsigned int controllerIndex,
			Controller::Button button,
			KeyState state,
			std::unique_ptr<Command> command);
		void UnbindControllerCommand(unsigned int controllerIndex,
			Controller::Button button,
			KeyState state);

		bool ProcessInput();

	private:
		friend class Singleton<InputManager>;
		InputManager() = default;

		static constexpr unsigned int MAX_CONTROLLERS{ 2 };

		struct KeyboardBinding
		{
			SDL_Scancode key;
			KeyState state;
			std::unique_ptr<Command> command;
		};
		std::vector<KeyboardBinding> m_keyboardBindings;

		struct ControllerBinding
		{
			unsigned int controllerIndex;
			Controller::Button button;
			KeyState state;
			std::unique_ptr<Command> command;
		};
		std::vector<ControllerBinding> m_controllerBindings;

		std::map<unsigned int, std::unique_ptr<Controller>> m_controllers;

		Controller* GetOrCreateController(unsigned int index);

		const bool* m_pKeyboardState{ nullptr };
		std::vector<Uint8> m_previousKeyboardState;
	};
}