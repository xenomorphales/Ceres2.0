/**
 * @file    position_manager.c
 * @author  David BITONNEAU <david.bitonneau@gmail.com>
 * @version V1.0
 * @date    12-Mar-2014
 * @brief   An odometry module. Implementation file.
 */

#include "PositionManager.h"
#include "control_system.h"
#include "QuadDecode_def.h"
#include <math.h>

PositionManager PositionManager::Instance;
QuadDecode<1> encoder1;  // Template using FTM1
QuadDecode<2> encoder2;  // Template using FTM2

//#define PI 3.1415926535f


#define position_ticks_to_mm(value_ticks) ((value_ticks) * 1000.0 / m_TicksPerM)

void PositionManager::Init(uint32_t ticks_per_m, double axle_track_mm) {
	encoder1.setup();
	encoder2.setup();
	encoder1.start();
	encoder2.start();
	
	m_TicksPerM = ticks_per_m;
	m_AxleTrackMm = axle_track_mm;

	m_LeftEncoder = 0;
	m_RightEncoder = 0;

	m_DistanceMm = 0;
	m_AngleRad = 0;

	m_XMm = 0;
	m_YMm = 0;
}

void PositionManager::Update()
{
	// Reading encoder value
  int32_t new_left_enc = encoder1.calcPosn();
  int32_t new_right_enc = -encoder2.calcPosn();
  
	int16_t left_enc_diff = m_LeftEncoder - new_left_enc;
	int16_t right_enc_diff = m_RightEncoder - new_right_enc;

	m_LeftEncoder = new_left_enc;
	m_RightEncoder = new_right_enc;

#if 0
	if (left_enc_diff != 0) {
		platform_led_toggle(PLATFORM_LED2);
		//printf("G.");//encG : %d   (sumG : %d)\n\r", (int)left_enc_diff, (int)m_LeftEncoder);
	}

	if (right_enc_diff != 0) {
		platform_led_toggle(PLATFORM_LED3);
		//printf("D.");//encD : %d   (sumD : %d)\n\r", (int)right_enc_diff, (int)m_RightEncoder);
	}
#endif

	// Distance
	double distance_diff_ticks = (left_enc_diff + right_enc_diff) / 2.0;
	double distance_diff_mm = position_ticks_to_mm(distance_diff_ticks);
	m_DistanceMm += distance_diff_mm;

	/*if (right_enc_diff != 0){
		printf("dist %d\n", (int)m_DistanceMm);
	}*/

	// Special case: no rotation
	if ((right_enc_diff - left_enc_diff) == 0) {
		m_XMm += -distance_diff_mm * sin(m_AngleRad);
		m_YMm += distance_diff_mm * cos(m_AngleRad);
		return;
	}

	double angle_diff_rad = atan(position_ticks_to_mm(right_enc_diff - left_enc_diff) / m_AxleTrackMm);

	// Special case: only rotation -> no need to update x and y
	if ((right_enc_diff + left_enc_diff) == 0) {
		m_AngleRad += angle_diff_rad;
		return;
	}

	// Radius of curvature
	double r = m_AxleTrackMm / 2.0 * (right_enc_diff + left_enc_diff) / (right_enc_diff - left_enc_diff);

	// Trajectory circle center coordinates
	double x0_mm = m_XMm - r * cos(m_AngleRad);
	double y0_mm = m_YMm - r * sin(m_AngleRad);

	// Update position
	m_AngleRad += angle_diff_rad;
	m_XMm = x0_mm + r * cos(m_AngleRad);
	m_YMm = y0_mm + r * sin(m_AngleRad);
}

void PositionManager::SetAxleTrackMm(double axle_track_mm)
{
	m_AxleTrackMm = axle_track_mm;
}

double PositionManager::GetAxleTrackMm(void) {
	return m_AxleTrackMm;
}

float PositionManager::GetLeftEncoder(void) {
	return m_LeftEncoder;
}

float PositionManager::GetRightEncoder(void) {
	return m_RightEncoder;
}

float PositionManager::GetDistanceMm(void) {
	return m_DistanceMm;
}

float PositionManager::GetAngleRad(void) {
	return m_AngleRad;
}

float PositionManager::GetAngleDeg(void) {
	return m_AngleRad * 180.f / PI;
}

void PositionManager::SetAngleDeg(float a){
	m_AngleRad = a * PI / 180.f;
	control_system_set_angle_rad_ref(m_AngleRad);
	control_system_reset_angle();
}

float PositionManager::GetXMm(void) {
	return m_XMm;
}

float PositionManager::GetYMm(void) {
	return m_YMm;
}

void PositionManager::SetXYMm(float x, float y){
	m_XMm = x;
	m_YMm = y;
}

int32_t PositionManager::MmToTicks(float value_mm) {
	return value_mm * m_TicksPerM / 1000.f;
}
