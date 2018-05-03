#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "Globals.h"

#define ENABLE_POSITIONNING 0
#define ENABLE_TIMER 0
#define ENABLE_AVOIDANCE 0

#define ROBOT_WIDTH 235.f
#define ROBOT_CENTER_BACK 55.f // distance between axle and back
#define ROBOT_CENTER_FRONT (150.f-ROBOT_CENTER_BACK) // distance between axle and front

enum class Side
{
	GREEN,
	ORANGE
};

enum class ArmState
{
	NORMAL,
	OPEN
};

enum class DoorState
{
	CLOSE,
	OPEN
};

class Strategy
{
public:
	enum class State
	{
		POSITIONING,
		POSITIONING2,
		WAITING_START,
		
		WATER_TOWER0,
		WATER_TOWER01,
		WATER_TOWER1,
		WATER_TOWER2,
		WATER_TOWER3,

		WATER_PLANT0,
		WATER_PLANT1,
		WATER_PLANT2,

		WAITING_END,
		END
	};
	
	static Strategy Instance;
	Strategy();
	void Init();
	void Task();
	void Start();
	
	void SetInitialPosition();
	void PushRobotAgainstWall(uint32_t durationMs = 1500, bool goForward = true);
	void RePosAgainstBackWall();
	void RePosAgainstWaterPlantSide();
	void RePosAgainstWaterPlantFront();

	Side GetSide() { return m_Side; }
	void SetSide(Side _side);
	void Print();

	Float2 GetCorrectPos(float x, float y);
	float GetCorrectAngle(float a);

	void SetArmState(ArmState _state);
	void SetDoorState(DoorState _state);

private:
	Side		m_Side = Side::GREEN;
	State		m_State = (State)0;
	uint32_t	m_StartTime = 0;
};

Strategy::State operator++(Strategy::State &s, int);

#endif
