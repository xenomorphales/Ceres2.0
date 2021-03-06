#include "MotorManager.h"
#include "Platform.h"
#include "PositionManager.h"
#include "ControlSystem.h"

const int motorPWMs[] = { 22, 23 };
const int motorDirs[] = { 26, 31 };

MotorManager MotorManager::Instance;

void MotorManager::Init()
{
	//setup IO
	for (unsigned i = 0; i < _countof(motorPWMs); ++i)
	{
		pinMode(motorPWMs[i], OUTPUT);
		analogWriteFrequency(motorPWMs[i], 15000.f);//PWM 15KHz
		pinMode(motorDirs[i], OUTPUT);
	}
	m_RightMotorPID.Init(0.f, 0.f, 0.f);
	m_LeftMotorPID.Init(0.f, 0.f, 0.f);
	SetMotorPidP(1.f);
	SetMotorPidD(0.25f);
}

void MotorManager::SetSpeed(MotorId m, int32_t speed)
{
	updateMotor(m, speed);
}

void MotorManager::SendCommand(MotorId m, int32_t cmd)
{
	if (!Enabled)
		cmd = 0;

	if (m == MotorManager::RIGHT)
		cmd = -cmd;
	
	int32_t AbsCmd = abs(cmd);
	if (AbsCmd < 2)
		AbsCmd = 0;
	else
		AbsCmd += 12;
	
	const int32_t maxi = 255;
	analogWrite(motorPWMs[m], (AbsCmd > maxi) ? maxi : AbsCmd);
	digitalWrite(motorDirs[m], (cmd >= 0) ? HIGH : LOW);
}

void MotorManager::updateMotor(MotorId m, int speed)
{
	//static double tt = -45;
	//if (m == LEFT)
	//	tt += 0.04;
	//if (tt > 0)
	//	speed = (m == RIGHT) ? -tt : tt;
	//else
	//	speed = 0;

	int32_t curEnc = (m == RIGHT) ? PositionManager::Instance.GetLeftEncoder() : PositionManager::Instance.GetRightEncoder(); // Yep it's not logic...
	auto & buffer = m_LastEncoder[m];
	float actualSpeed = 0;
	if (buffer.GetSize())
	{
		actualSpeed = (curEnc - m_LastEncoder[m].Back()) / (float)buffer.GetSize();
	}
	if (buffer.IsFull())
		buffer.PopBack();
	buffer.PushFront(curEnc);
	
	float err = (speed - actualSpeed);// *(1.f / CONTROL_SYSTEM_PERIOD_S);

	int cmd;
	if (m == RIGHT)
	{
		cmd = m_RightMotorPID.EvaluatePID(err);
	}
	else
	{
		cmd = m_LeftMotorPID.EvaluatePID(err);
	}

	//if (tt >= 0)
	//{
	//	if (m == LEFT)
	//		Serial.printf("%d %d %f ", speed, cmd, actualSpeed);
	//	else
	//		Serial.printf("%d %f\r\n", -cmd, -actualSpeed);
	//}
	SendCommand(m, cmd);
}

