#include <WProgram.h>
#include <EEPROM.h>
#include "Strategy.h"
#include "Platform.h"
#include "TrajectoryManager.h"
#include "PositionManager.h"
#include "MotorManager.h"

#define ENABLE_TIMER 0
#define ENABLE_AVOIDANCE 0

Strategy Strategy::Instance;

Strategy::Strategy()
{
}

void Strategy::Init()
{
	eeprom_initialize();
	eeprom_busy_wait();
	if (eeprom_read_byte(0) == (uint8_t)Side::BLUE)
		m_Side = Side::BLUE;
	else
		m_Side = Side::YELLOW;
}

void Strategy::Task()
{
	if (m_State == State::WAITING_START)
	{
#if ENABLE_TIMER
		if (Platform::IsStartPulled())
			Start();
#endif
		return;
	}

#if ENABLE_TIMER
	if (millis() > m_StartTime + 90000)
	{
		if (m_State != State::END)
		{
			// do funny action
			m_State = State::END;
		}
		TrajectoryManager::Instance.Pause();
		return;
	}
#endif

#if ENABLE_AVOIDANCE
	if (Platform::IsGP2Occluded())
	{
		TrajectoryManager::Instance.Pause();
		return;
	}
	else
	{
		TrajectoryManager::Instance.Resume();
	}
#endif

	if (!TrajectoryManager::Instance.IsEnded())
		return;

	switch (m_State)
	{
	case State::MODULE_A1:
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(1200.f, 600.f));
		TrajectoryManager::Instance.GotoDegreeAngle(GetCorrectAngle(90.f));
		TrajectoryManager::Instance.GotoDistance(250.f);
		break;

	case State::MODULE_A2:
		SetGripState(GripState::CLOSE);
		SetArmState(ArmState::EMPTYING);
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(775, 790.f));
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(250, 775.f));
		break;

	case State::MODULE_A3:
		PushRobotAgainstWall();
		SetGripState(GripState::FULLY_OPEN);
		TrajectoryManager::Instance.GotoDistance(-100.f);
		break;

	case State::MODULE_B1:
		SetGripState(GripState::NORMAL);
		SetArmState(ArmState::NORMAL);
		SetGripState(GripState::FULLY_OPEN);
		break;
	
	case State::MODULE_B2:
		TrajectoryManager::Instance.GotoDegreeAngle(GetCorrectAngle(150.f));
		TrajectoryManager::Instance.GotoDistance(120.f);
		break;

	case State::MODULE_B3:
		SetGripState(GripState::CLOSE);
		SetArmState(ArmState::EMPTYING);
		TrajectoryManager::Instance.GotoDistance(-200.f);
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(400.f, 910.f));
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(250.f, 910.f));
		break;

	case State::MODULE_B4:
		PushRobotAgainstWall();
		SetGripState(GripState::FULLY_OPEN);
		TrajectoryManager::Instance.GotoDistance(-100.f);
		break;

	default:
		return;
	}

	if (m_State < State::WAITING_END)
	{
		m_State++;
		Serial.printf("new state: %d", (int)m_State);
	}
}

void Strategy::Start()
{
	if (m_State == State::WAITING_START)
	{
		m_State++;
		m_StartTime = millis();
		SetArmState(ArmState::NORMAL);
		SetGripState(GripState::FULLY_OPEN);
	}
}

void Strategy::SetInitialPosition()
{
	float y = 360.f - ROBOT_CENTER_FRONT;
	if (m_Side == Side::BLUE)
	{
		PositionManager::Instance.SetPosMm(Float2(1070.f - 0.5f * ROBOT_WIDTH, y));
	}
	else
	{
		PositionManager::Instance.SetPosMm(Float2(1930.f + 0.5f * ROBOT_WIDTH, y));
	}
}

void Strategy::PushRobotAgainstWall()
{
	ControlSystem::Instance.m_Enable = false;
	SendCommandToMotor(RIGHT_MOTOR, -25);
	SendCommandToMotor(LEFT_MOTOR, -25);
	delay(1000);
	SendCommandToMotor(RIGHT_MOTOR, 0);
	SendCommandToMotor(LEFT_MOTOR, 0);
	ControlSystem::Instance.Reset();
	ControlSystem::Instance.m_Enable = true;
}

void Strategy::Print()
{
	Serial.print("State: ");
	switch (m_State)
	{
	case State::WAITING_START:
		Serial.print("waiting start\r\n");
		break;
	case State::WAITING_END:
		Serial.print("waiting end\r\n");
		break;
	case State::END:
		Serial.print("end\r\n");
		break;
	default:
		Serial.printf("actions (%d)\r\n", (int)m_State);
		break;
	};
	Serial.printf("Side: %s\r\n", m_Side == Side::BLUE ? "Blue" : "Yellow");
	Serial.printf("Time since start: %ds\r\n", (millis() - m_StartTime) / 1000);
	Serial.printf("Is trajectory paused: %d\r\n", (int)TrajectoryManager::Instance.IsPaused());
}

void Strategy::SetSide(Side _side)
{
	m_Side = _side;
	eeprom_busy_wait();
	eeprom_write_byte(0, (uint8_t)m_Side);
	Serial.println("set initial pos");
	SetInitialPosition();
}

Float2 Strategy::GetGameElementPosition(GameElement _module)
{
	if (m_Side == Side::BLUE)
	{
		if (_module == GameElement::MODULE_A)
			return Float2(1000.f, 600.f);
		if (_module == GameElement::MODULE_B)
			return Float2(200.f, 600.f);
		if (_module == GameElement::MODULE_C)
			return Float2(500.f, 1100.f);
		if (_module == GameElement::MODULE_D)
			return Float2(900.f, 1400.f);
		if (_module == GameElement::MODULE_E)
			return Float2(800.f, 1850.f);
	}
	else
	{
		if (_module == GameElement::MODULE_A)
			return Float2(2000.f, 600.f);
		if (_module == GameElement::MODULE_B)
			return Float2(2800.f, 600.f);
		if (_module == GameElement::MODULE_C)
			return Float2(2500.f, 1100.f);
		if (_module == GameElement::MODULE_D)
			return Float2(2100.f, 1400.f);
		if (_module == GameElement::MODULE_E)
			return Float2(2200.f, 1850.f);
	}
	Serial.println("incorrect game element");
	return Float2();
}

Float2 Strategy::GetCorrectPos(float x, float y)
{
	if (m_Side == Side::BLUE)
		return Float2(x, y);
	else
		return Float2(3000.f - x, y);
}

float Strategy::GetCorrectAngle(float a)
{
	if (m_Side == Side::BLUE)
		return a;
	else
		return -a;
}

void Strategy::SetArmState(ArmState _state)
{
	switch (_state)
	{
	case ArmState::NORMAL:
		Platform::SetServoPos(ServoID::SERVO1, 825);
		break;
	case ArmState::EMPTYING:
		Platform::SetServoPos(ServoID::SERVO1, 500);
		break;
	}
	delay(1000);
}

void Strategy::SetGripState(GripState _state)
{
	switch (_state)
	{
	case GripState::CLOSE:
		Platform::SetServoPos(ServoID::SERVO2, 300);
		break;
	case GripState::NORMAL:
		Platform::SetServoPos(ServoID::SERVO2, 350);
		break;
	case GripState::FULLY_OPEN:
		Platform::SetServoPos(ServoID::SERVO2, 400);
		break;
	}
	delay(1000);
}

Strategy::State operator++(Strategy::State &s, int)
{
	if (s != Strategy::State::END)
		s = (Strategy::State)((int)s + 1);
	return s;
}
