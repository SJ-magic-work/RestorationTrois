/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"

#include "sj_common.h"
// #include "util.h"

/************************************************************
************************************************************/
class DRAW_OFFSET_MANAGER{
private:
	enum STATE{
		STATE__GODOWN,
		STATE__RECOVER_FROM_OVERSHOOT,
	};
	
	STATE State;
	double offset;
	double offset_total;
	
	double OfsTarget;
	double OfsTarget_Over;
	double OfsTarget_SpeedDown;
	
	double speed; // pix/sec
	const double speed_min;
	const double speed_max;
	const double accel;
	const double accel_OverShoot;
	double sgn;
	
	int LastInt;
	
public:
	DRAW_OFFSET_MANAGER()
	: offset(0)
	, speed_min(140)
	, speed_max(500)
	, accel((500 - 140)/1.5)
	, accel_OverShoot(2.0 * 140.0 / 0.35)
	{
	}
	
	void start(int now, double _OfsTarget, double _OfsTarget_Over, double _OfsTarget_SpeedDown)
	{
		State = STATE__GODOWN;
		
		offset_total = 0;
		offset = 0;
		
		speed = speed_min;
		sgn = 1;
		LastInt = now;
		
		OfsTarget = _OfsTarget;
		OfsTarget_Over = _OfsTarget_Over;
		OfsTarget_SpeedDown = _OfsTarget_SpeedDown;
	}
	
	/******************************
	ret
		0: none.
		1: reached "Height" and canceled offset. please change contents id.
		2: Finished.
	******************************/
	int update(int now, float Height)
	{
		int ret = 0;
		
		switch(State){
			case STATE__GODOWN:
			{
				speed = speed + sgn * accel * (double(now - LastInt) * 1.0e-3);
				if(speed < speed_min) speed = speed_min;
				if(speed_max < speed) speed = speed_max;
				
				double _ofs = speed * (double(now - LastInt) * 1.0e-3);
				offset += _ofs;
				offset_total += _ofs;
				
				if(Height <= offset){
					offset -= Height;
					ret = 1;
				}
				
				if( (0 < sgn) && (OfsTarget_SpeedDown <= offset_total)) sgn = -1.0;
				
				if(OfsTarget_Over <= offset_total){
					State = STATE__RECOVER_FROM_OVERSHOOT;
				}
			}
				break;
				
			case STATE__RECOVER_FROM_OVERSHOOT:
			{
				speed = speed - accel_OverShoot * (double(now - LastInt) * 1.0e-3);
				if(speed < -speed_min) speed = -speed_min;
				
				offset += speed * (double(now - LastInt) * 1.0e-3);
				if(offset <= 0) ret = 2;
			}
				break;
		}
		
		LastInt = now;
		
		return ret;
	}
	
	void StartSpeedDown() { sgn = -1; }
	
	double get_ofs() { return offset; }
};
