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
	double offset;
	double speed; // pix/sec
	const double speed_min;
	const double speed_max;
	const double accel;
	double sgn;
	
	int LastInt;
	
public:
	DRAW_OFFSET_MANAGER()
	: offset(0)
	, speed_min(140)
	, speed_max(500)
	, accel((500 - 140)/1.5)
	{
	}
	
	void start(int now)
	{
		offset = 0;
		speed = speed_min;
		sgn = 1;
		LastInt = now;
	}
	
	bool update(int now, float Height)
	{
		bool b_ReachedTop = false;
		
		speed = speed + sgn * accel * (double(now - LastInt) * 1.0e-3);
		if(speed < speed_min) speed = speed_min;
		if(speed_max < speed) speed = speed_max;
		
		offset += speed * (double(now - LastInt) * 1.0e-3);
		
		if(Height <= offset){
			offset -= Height;
			b_ReachedTop = true;
		}
		
		LastInt = now;
		
		return b_ReachedTop;
	}
	
	void StartSpeedDown() { sgn = -1; }
	
	double get_ofs() { return offset; }
};
