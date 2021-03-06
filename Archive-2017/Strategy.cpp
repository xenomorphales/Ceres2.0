#include <WProgram.h>
#include <EEPROM.h>
#include "Strategy.h"
#include "Platform.h"
#include "TrajectoryManager.h"
#include "PositionManager.h"
#include "MotorManager.h"

Strategy Strategy::Instance;

Strategy::Strategy()
{
}

void Strategy::Init()
{
#if ENABLE_POSITIONNING
	m_State = State::POSITIONING;
#else
	m_State = State::WAITING_START;
#endif
	eeprom_initialize();
	eeprom_busy_wait();
	//if (eeprom_read_byte(0) == (uint8_t)Side::BLUE)
	if (Platform::IsButtonPressed(1))
		m_Side = Side::BLUE;
	else
		m_Side = Side::YELLOW;
}

void Strategy::Task()
{
#if ENABLE_POSITIONNING
	if (m_State == State::POSITIONING)
	{
		if (Platform::IsStartPulled())
		{
			TrajectoryManager::Instance.GotoXY(Float2(PositionManager::Instance.GetXMm(), 360.f - ROBOT_CENTER_FRONT - 30.f));
			m_State++;
		}
	}
	if (m_State == State::POSITIONING2 && !Platform::IsStartPulled())
	{
		delay(500);
		m_State++;
	}
#endif

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
		if (m_State != State::END && (millis() > m_StartTime + 91000))
		{
			SetFunnyState(FunnyState::EJECT);

			m_State = State::END;
		}
		TrajectoryManager::Instance.Pause();
		MotorManager::Instance.Enabled = false;
		return;
	}
#endif

#if ENABLE_AVOIDANCE
	if (Platform::IsGP2Occluded(TrajectoryManager::Instance.IsForwardMovement()))
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
	case State::MODULE_A0:
		TrajectoryManager::Instance.GotoDistance(50.f);
		break;

	case State::MODULE_A1:
		if (m_Side == Side::YELLOW)
			TrajectoryManager::Instance.GotoXY(GetCorrectPos(1200.f, 600.f));
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(1000.f, 600.f));
		break;

	case State::MODULE_A2:
		SetGripState(GripState::CLOSE);
		SetArmState(ArmState::EMPTYING);
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(775, 790.f));
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(250, 775.f));
		break;

	case State::MODULE_A3:
		PushRobotAgainstWall();
		RePosAgainstSideBase();
		SetGripState(GripState::FULLY_OPEN);
		TrajectoryManager::Instance.GotoDistance(-100.f);
		break;

	case State::MODULE_B1:
		SetGripState(GripState::NORMAL);
		SetArmState(ArmState::NORMAL);
		SetGripState(GripState::FULLY_OPEN);
		break;
	
	case State::MODULE_B2:
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(200, 600.f));
		break;

	case State::MODULE_B3:
		SetGripState(GripState::CLOSE);
		SetArmState(ArmState::EMPTYING);
		TrajectoryManager::Instance.GotoDistance(-200.f);
		break;

	case State::MODULE_B4:
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(400.f, 920.f));
		break;

	case State::MODULE_B5:
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(250.f, 920.f));
		break;

	case State::MODULE_B6:
		PushRobotAgainstWall();
		RePosAgainstSideBase();
		SetGripState(GripState::FULLY_OPEN);
		if (m_Side == Side::BLUE)
		{
			TrajectoryManager::Instance.GotoDistance(-200.f);
		}
		else
		{
			TrajectoryManager::Instance.GotoDistance(-700.f);
		}
		break;

	case State::MODULE_C1:
		SetGripState(GripState::NORMAL);
		SetArmState(ArmState::NORMAL);
		SetGripState(GripState::FULLY_OPEN);
		if (m_Side == Side::YELLOW)
			TrajectoryManager::Instance.GotoXY(GetCorrectPos(700.f, 1120.f));
		break;

	case State::MODULE_C2:
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(500.f, 1120.f));
		break;

	case State::MODULE_C3:
		SetGripState(GripState::CLOSE);
		SetArmState(ArmState::EMPTYING);
		if (m_Side == Side::YELLOW)
			TrajectoryManager::Instance.GotoXY(GetCorrectPos(250.f, 1035.f));
		else
			TrajectoryManager::Instance.GotoXY(GetCorrectPos(250.f, 1045.f));
		TrajectoryManager::Instance.GotoDegreeAngle(GetCorrectAngle(90.f));
		break;

	case State::MODULE_C4:
		PushRobotAgainstWall();
		RePosAgainstSideBase();
		SetGripState(GripState::FULLY_OPEN);
		TrajectoryManager::Instance.GotoDistance(-150.f);
		break;

#if ENABLE_EXTRA
	case State::MODULE_RECAL0:
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(202.f + 150.f, 360.f + 200.f));
		TrajectoryManager::Instance.GotoDegreeAngle(GetCorrectAngle(0.f));
		break;

	case State::MODULE_RECAL1:
		PushRobotAgainstWall(2000, false);
		RePosAgainstFrontBase();
		TrajectoryManager::Instance.GotoDistance(150.f);
		break;
#endif

	case State::MODULE_E1:
		SetGripState(GripState::NORMAL);
		SetArmState(ArmState::NORMAL);
		SetGripState(GripState::FULLY_OPEN);
		TrajectoryManager::Instance.GotoXY(GetCorrectPos(800.f, 1850.f));
		break;

	case State::MODULE_E2:
		SetGripState(GripState::CLOSE);
		SetArmState(ArmState::EMPTYING);
		TrajectoryManager::Instance.GotoDistance(-100.f);
		TrajectoryManager::Instance.GotoDegreeAngle(GetCorrectAngle(-135.f));
		break;

	case State::MODULE_E3:
		PushRobotAgainstWall(3000);
		SetGripState(GripState::FULLY_OPEN);
		delay(500);
		TrajectoryManager::Instance.GotoDistance(-100.f);
		break;

	case State::MODULE_E4:
		PushRobotAgainstWall();
		break;

	default:
		return;
	}

	if (m_State < State::WAITING_END)
	{
		m_State++;
		Serial.printf("new state: %d\r\n", (int)m_State);
	}
}

void Strategy::Start()
{
	if (m_State == State::WAITING_START)
	{
		m_State++;
		m_StartTime = millis();
		Platform::InitServo();
		SetArmState(ArmState::NORMAL);
		SetGripState(GripState::FULLY_OPEN);
	}
}

void Strategy::SetInitialPosition()
{
	float y = ROBOT_CENTER_BACK;//360.f - ROBOT_CENTER_FRONT
	if (m_Side == Side::BLUE)
	{
		PositionManager::Instance.SetPosMm(Float2(1070.f - 0.5f * ROBOT_WIDTH, y));
	}
	else
	{
		PositionManager::Instance.SetPosMm(Float2(1930.f + 0.5f * ROBOT_WIDTH, y));
	}
}

void Strategy::PushRobotAgainstWall(uint32_t durationMs, bool goForward)
{
	ControlSystem::Instance.m_Enable = false;
	int cmd = -30;
	if (!goForward)
		cmd = -cmd;
	MotorManager::Instance.SetSpeed(MotorManager::RIGHT, cmd);
	MotorManager::Instance.SetSpeed(MotorManager::LEFT, cmd);
	delay(durationMs);
	MotorManager::Instance.SetSpeed(MotorManager::RIGHT, 0);
	MotorManager::Instance.SetSpeed(MotorManager::LEFT, 0);
	ControlSystem::Instance.Reset();
	ControlSystem::Instance.m_Enable = true;
}

void Strategy::RePosAgainstSideBase()
{
	PositionManager::Instance.SetAngleDeg(GetCorrectAngle(90.f));
	PositionManager::Instance.SetPosMm(GetCorrectPos(202.f, PositionManager::Instance.GetPosMm().y));
}

void Strategy::RePosAgainstFrontBase()
{
	PositionManager::Instance.SetAngleDeg(GetCorrectAngle(0.f));
	PositionManager::Instance.SetPosMm(Float2(PositionManager::Instance.GetPosMm().x, 382.f + ROBOT_CENTER_BACK));
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
		Platform::SetServoPos(ServoID::SERVO2, 310);
		break;
	case GripState::NORMAL:
		Platform::SetServoPos(ServoID::SERVO2, 350);
		break;
	case GripState::FULLY_OPEN:
		Platform::SetServoPos(ServoID::SERVO2, 400);
		break;
	}
	delay(500);
}

void Strategy::SetFunnyState(FunnyState _state)
{
	switch (_state)
	{
		case FunnyState::NORMAL:
			Platform::SetServoPos(ServoID::SERVO3, 250);
			break;

		case FunnyState::EJECT:
			Platform::SetServoPos(ServoID::SERVO3, 0);
			break;
	}
}

Strategy::State operator++(Strategy::State &s, int)
{
	if (s != Strategy::State::END)
		s = (Strategy::State)((int)s + 1);
	return s;
}
