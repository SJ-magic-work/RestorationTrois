/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"

#include "sj_common.h"
#include "util.h"

/************************************************************
************************************************************/
	
/**************************************************
**************************************************/
class STATE_REPAIR{
public:
	enum STATE{
		STATE__STABLE,
		STATE__REPAIR,
		STATE__RAPID,
		STATE__BREAK,
		STATE__BUFFER,
		STATE__WAIT_POWER_REPAIR,
		STATE__POWER_REPAIR,
		STATE__SHORT_REPAIR,
		
		NUM_STATE,
	};
	
private:
	ofSoundPlayer sound_Pi;
	ofSoundPlayer sound_Rapid;
	ofSoundPlayer sound_PowerRepair;
	ofSoundPlayer sound_ShortRepair;
	
	const float MaxVol_Sound_PowerRepair;
	const float MaxVol_Sound_ShortRepair;
	
	int t_LastSound_Pi;
	
	STATE State;
	int t_from_ms;
	int duration;
	const int duration_shortRepair;
	int LastInt;
	
	bool b_DispCursor;
	int c_Rapid;
	
	ofVec2f Cross;
	ofVec2f Cross_from;
	ofVec2f Cross_Next;
	ofVec2f dir;
	float SquareDistance;
	const float RepairSpeed_PixPerSec;
	
	float PatchSize;
	

public:
	STATE_REPAIR()
	: State(STATE__STABLE)
	, b_DispCursor(false)
	, LastInt(0)
	, t_LastSound_Pi(0)
	, Cross(ofVec2f(0, 0))
	, RepairSpeed_PixPerSec(140)
	, PatchSize(50)
	, MaxVol_Sound_PowerRepair(1.0)
	, MaxVol_Sound_ShortRepair(0.8)
	, duration_shortRepair(1500)
	{
		SJ_UTIL::setup_sound(sound_Pi, "sound/Repair/pi_1.mp3", false, 0.05);
		SJ_UTIL::setup_sound(sound_Rapid, "sound/Repair/Cash_Register-Beep01-1.mp3", false, 0.05);
		
		SJ_UTIL::setup_sound(sound_PowerRepair, "sound/Repair/computer-keyboard-type-long_GJE_D24u.mp3", true, 0.0);
		sound_PowerRepair.play();
		
		SJ_UTIL::setup_sound(sound_ShortRepair, "sound/Repair/computer-keyboard-type-keys-typing_G1N16KNd.mp3", true, 0.0);
		sound_ShortRepair.play();
	}
	
	void setup(bool b_mov){
	}
	
	void Transition(int NextState, int now)
	{
		/* */
		if((NextState < 0) || (NUM_STATE <= NextState)) return;
		
		/* */
		State = STATE(NextState);
		t_from_ms = now;
		
		switch(State){
			case STATE__STABLE:
				b_DispCursor = false;
				sound_PowerRepair.setVolume(0.0);
				sound_ShortRepair.setVolume(0.0);
				break;
				
			case STATE__REPAIR:
				b_DispCursor = true;
				if(10e+3 < now - t_LastSound_Pi) { sound_Pi.play(); t_LastSound_Pi = now; }
				break;
				
			case STATE__RAPID:
				duration = 80;
				c_Rapid = 0;
				break;
				
			case STATE__BREAK:
				duration = 50;
				break;
				
			case STATE__BUFFER:
				duration = 500;
				b_DispCursor = false;
				sound_PowerRepair.setVolume(0.0);
				sound_ShortRepair.setVolume(0.0);
				break;
				
			case STATE__WAIT_POWER_REPAIR:
				duration = 1500;
				break;
				
			case STATE__POWER_REPAIR:
				duration = (int)ofRandom(2500, 3000);
				sound_PowerRepair.setVolume(MaxVol_Sound_PowerRepair);
				break;
				
			case STATE__SHORT_REPAIR:
				sound_ShortRepair.setVolume(MaxVol_Sound_ShortRepair);
				break;
				
			default:
				break;
		}
	}
	
	STATE get_State(){ return State; }
	
	void Set_PatchSize(float Rest){
		// float _size = Rest / 5;
		float _size = Rest / 4;
		_size = sqrt(_size);
		
		if(_size < 50) _size = 50;
		PatchSize = _size;
	}
	
	bool IsTimeout(int now){
		if(duration < now - t_from_ms)	return true;
		else							return false;
	}
	
	void SetNextTarget(ofVec2f _NextTarget){
		Cross_Next = _NextTarget;
		
		SquareDistance = Cross.squareDistance(Cross_Next);
		
		dir = Cross_Next - Cross;
		dir.normalize();
		
		Cross_from = Cross;
	}
	
	void update(int now){
		/********************
		********************/
		if(State == STATE__REPAIR){
			ofVec2f NextPos = Cross + dir * (RepairSpeed_PixPerSec * (now - LastInt) * 1e-3);
			if(SquareDistance <= Cross_from.squareDistance(NextPos))	Cross = Cross_Next;
			else														Cross = NextPos;
		}
		
		/********************
		********************/
		LastInt = now;
	}
	
	/******************************
	when ret == true : Please Change Next Target.
	******************************/
	bool draw(int now, ofFbo& fbo){
		bool ret = false;
		
		if(State == STATE__REPAIR){
			if(Cross == Cross_Next){
				State = STATE__BREAK;
				
				fbo.begin();
				ofSetColor(255, 255, 255, 255);
				ofDrawRectangle(Cross.x - PatchSize/2, Cross.y - PatchSize/2, PatchSize, PatchSize);
				fbo.end();
				
				sound_Pi.play();
				t_LastSound_Pi = now;
				
				Transition(STATE__BREAK, now);
			}
			
		}else if(State == STATE__RAPID){
			if(duration < now - t_from_ms){
				t_from_ms = now;
				c_Rapid++;
				Cross = Cross_Next;
				
				fbo.begin();
				ofSetColor(255, 255, 255, 255);
				ofDrawRectangle(Cross.x - PatchSize/2, Cross.y - PatchSize/2, PatchSize, PatchSize);
				fbo.end();
				
				sound_Rapid.play();
				
				ret = true;
			}
		}else if(State == STATE__SHORT_REPAIR){
			fbo.begin();
			ofSetColor(255, 255, 255, 255);
			
			float _x = get_CursorPos_ShortRepair(now, fbo);
			ofDrawRectangle(0, 0, _x, fbo.getHeight());
			fbo.end();
			
			if(duration_shortRepair <= now - t_from_ms){
				Transition(STATE__BUFFER, now);
			}
		}
		
		return ret;
	}
	
	float get_PatchSize(){ return PatchSize; }
	bool IsDispCursor(){ return b_DispCursor; }
	int get__c_Rapid(){ return c_Rapid; }
	ofVec2f get_CursorPos(){ return Cross; }
	ofVec2f get_TargetCursorPos(){ return Cross_Next; }
	float get_CursorPos_ShortRepair(int now, ofFbo& fbo) { return float(fbo.getWidth()) / duration_shortRepair * float(now - t_from_ms); }
};