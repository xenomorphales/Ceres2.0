/**
 ********************************************************************
 * @file    pid.h
 * @author  Kevin JOLY
 * @author  David BITONNEAU <david.bitonneau@gmail.com>
 * @version V1.1
 * @date    14-Mar-2014
 * @brief   PID controller definition file. Contain controllers
 *          for control engineering.
 ********************************************************************
 * @attention
 *
 * This file is part of LIBAUSBEE.
 *
 * LIBAUSBEE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LIBAUSBEE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LIBAUSBEE.  If not, see <http://www.gnu.org/licenses/>.
 *
 * <h2><centor>&copy;  Copyright 2013-2014 (C) EIRBOT </center></h2>
 ********************************************************************
 */
#ifndef PID_H
#define PID_H

#include <stdint.h>

/**
* @class PIDController
* @brief PID controller structure
*
* PIDController contains all the parameters and status of the PID controller.
*
*/
class PIDController 
{
public:
	void Init(float Kp, float Ki, float Kd);
	void SetOutputRange(float max_output);

	void SetKP(float Kp);
	void SetKI(float Ki);
	void SetKD(float Kd);

	float GetKP();
	float GetKI();
	float GetKD();

	float GetError();
	float GetErrorSum();
	float GetErrorDiff();

	float EvaluatePID(float error);

private:
	float m_Kp; /*!< Proportional value. */
	float m_Ki; /*!< Integral value. */
	float m_Kd; /*!< Derivative value. */

	float m_last_error; /*!< Previous error observed. */
	float m_error_sum; /*!< Sum of previous errors. */
	float m_error_diff; /*!< Diff with previous errors. */

	float m_max_output; /*!< Maximum saturation output value. */
};

#endif
