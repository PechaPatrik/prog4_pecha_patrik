#include "Controller.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

namespace dae
{
	class Controller::ControllerImpl
	{
	public:
		explicit ControllerImpl(unsigned int index)
			: m_controllerIndex(index)
		{
			ZeroMemory(&m_previousState, sizeof(XINPUT_STATE));
			ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
		}

		void Update()
		{
			m_previousState = m_currentState;
			ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
			XInputGetState(m_controllerIndex, &m_currentState);

			const WORD prev = m_previousState.Gamepad.wButtons;
			const WORD cur = m_currentState.Gamepad.wButtons;

			m_buttonsPressedThisFrame = (cur ^ prev) & cur;
			m_buttonsReleasedThisFrame = (cur ^ prev) & prev;
		}

		bool IsDown(unsigned int button) const { return (m_buttonsPressedThisFrame & button) != 0; }
		bool IsUp(unsigned int button) const { return (m_buttonsReleasedThisFrame & button) != 0; }
		bool IsPressed(unsigned int button) const { return (m_currentState.Gamepad.wButtons & button) != 0; }

	private:
		unsigned int m_controllerIndex{};
		XINPUT_STATE m_previousState{};
		XINPUT_STATE m_currentState{};
		WORD m_buttonsPressedThisFrame{};
		WORD m_buttonsReleasedThisFrame{};
	};

	Controller::Controller(unsigned int controllerIndex)
		: m_pImpl(std::make_unique<ControllerImpl>(controllerIndex))
	{
	}

	Controller::~Controller() = default;

	void Controller::Update()
	{
		m_pImpl->Update();
	}

	bool Controller::IsDown(Button button) const
	{
		return m_pImpl->IsDown(static_cast<unsigned int>(button));
	}

	bool Controller::IsUp(Button button) const
	{
		return m_pImpl->IsUp(static_cast<unsigned int>(button));
	}

	bool Controller::IsPressed(Button button) const
	{
		return m_pImpl->IsPressed(static_cast<unsigned int>(button));
	}
}