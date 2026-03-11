#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include "InputManager.h"

#include <algorithm>

namespace dae
{
	void InputManager::BindKeyboardCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command)
	{
		m_keyboardBindings.push_back({ key, state, std::move(command) });
	}

	void InputManager::UnbindKeyboardCommand(SDL_Scancode key, KeyState state)
	{
		m_keyboardBindings.erase(
			std::remove_if(m_keyboardBindings.begin(), m_keyboardBindings.end(),
				[key, state](const KeyboardBinding& b)
				{
					return b.key == key && b.state == state;
				}),
			m_keyboardBindings.end());
	}

	void InputManager::BindControllerCommand(unsigned int controllerIndex,
		Controller::Button button,
		KeyState state,
		std::unique_ptr<Command> command)
	{
		GetOrCreateController(controllerIndex); // ensure it exists
		m_controllerBindings.push_back({ controllerIndex, button, state, std::move(command) });
	}

	void InputManager::UnbindControllerCommand(unsigned int controllerIndex,
		Controller::Button button,
		KeyState state)
	{
		m_controllerBindings.erase(
			std::remove_if(m_controllerBindings.begin(), m_controllerBindings.end(),
				[controllerIndex, button, state](const ControllerBinding& b)
				{
					return b.controllerIndex == controllerIndex &&
						b.button == button &&
						b.state == state;
				}),
			m_controllerBindings.end());
	}

	Controller* InputManager::GetOrCreateController(unsigned int index)
	{
		if (index >= MAX_CONTROLLERS)
			return nullptr;

		auto it = m_controllers.find(index);
		if (it == m_controllers.end())
		{
			m_controllers[index] = std::make_unique<Controller>(index);
			it = m_controllers.find(index);
		}
		return it->second.get();
	}

	bool InputManager::ProcessInput()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
				return false;
			ImGui_ImplSDL3_ProcessEvent(&e);
		}

		int numKeys{};
		m_pKeyboardState = SDL_GetKeyboardState(&numKeys);

		if (m_previousKeyboardState.empty())
			m_previousKeyboardState.assign(numKeys, false);

		for (const auto& binding : m_keyboardBindings)
		{
			const bool curDown = m_pKeyboardState[binding.key];
			const bool prevDown = m_previousKeyboardState[binding.key];

			bool shouldExecute = false;
			switch (binding.state)
			{
			case KeyState::Down: 
				shouldExecute = curDown && !prevDown; 
				break;
			case KeyState::Up: 
				shouldExecute = !curDown && prevDown; 
				break;
			case KeyState::Pressed: 
				shouldExecute = curDown; 
				break;
			}

			if (shouldExecute)
				binding.command->Execute();
		}

		m_previousKeyboardState.assign(m_pKeyboardState, m_pKeyboardState + numKeys);

		for (auto& [index, controller] : m_controllers)
			controller->Update();

		for (const auto& binding : m_controllerBindings)
		{
			auto it = m_controllers.find(binding.controllerIndex);
			if (it == m_controllers.end()) continue;

			const Controller* ctrl = it->second.get();
			bool shouldExecute = false;
			switch (binding.state)
			{
			case KeyState::Down: 
				shouldExecute = ctrl->IsDown(binding.button); 
				break;
			case KeyState::Up: 
				shouldExecute = ctrl->IsUp(binding.button); 
				break;
			case KeyState::Pressed: 
				shouldExecute = ctrl->IsPressed(binding.button); 
				break;
			}

			if (shouldExecute)
				binding.command->Execute();
		}

		return true;
	}
}