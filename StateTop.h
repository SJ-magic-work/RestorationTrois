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
class STATE_TOP{
public:
	enum STATE{
		STATE__SLEEP,
		STATE__SLEEP_PLAYING,
		STATE__RUN,
		STATE__CHANGING_CONTENTS,
		STATE__WAIT_STABLE,
		
		NUM_STATE,
	};
	
private:
	int LastInt;
	
	ofSoundPlayer sound_Noise;
	float MaxVol_SoundNoise;
	float d_VolFade;
	
	int t_from_ms;
	int duration;
	
	STATE State;

	bool _IsTimeout(int now){
		if( (State == STATE__SLEEP) || (State == STATE__SLEEP_PLAYING) || (State == STATE__CHANGING_CONTENTS) ) return false;
		
		if(duration < now - t_from_ms)	return true;
		else							return false;
	}
	
	bool _IsTimeout(ofVideoPlayer* video){
		/********************
		********************/
		if(!video->isLoaded() || !video->isPlaying())						return false;
		if( (State == STATE__SLEEP) || (State == STATE__SLEEP_PLAYING) )	return false;
		
		/********************
		********************/
		int TotalNumFrames = video->getTotalNumFrames();
		int CurrentFrame = video->getCurrentFrame();
		
		switch(State){
			case STATE__RUN:
				if(TotalNumFrames - 20 < CurrentFrame)	return true;
				else									return false;
				break;
				
			case STATE__CHANGING_CONTENTS:
				if(TotalNumFrames - 5 < CurrentFrame)	return true;
				else									return false;
				break;
				
			case STATE__WAIT_STABLE:
				if(5 < CurrentFrame)	return true;
				else					return false;
				break;
				
			default:
				break;
		}
		
		return false;
	}
	
public:
	STATE_TOP()
	: State(STATE__SLEEP)
	, duration(2000) // temporary
	, MaxVol_SoundNoise(0.8)
	, d_VolFade(1.0)
	, LastInt(0)
	{
	}
	
	void setup(bool b_mov){
		if(b_mov)	SJ_UTIL::setup_sound(sound_Noise, "sound/top/radio-tuning-noise-short-waves_zydE-HEd.mp3", true, 0.0);
		else		SJ_UTIL::setup_sound(sound_Noise, "sound/top/room-tones-movie-film-projection-room_MJdPiL4d.wav", true, 0.0);
		
		sound_Noise.play();
	}
	
	void update(int now){
		float vol = sound_Noise.getVolume();
		float VolFadeSpeed = (MaxVol_SoundNoise - 0) / d_VolFade;
		
		if(State == STATE__CHANGING_CONTENTS){
			vol += VolFadeSpeed * float(now - LastInt) * 1.0e-3;
		}else{
			vol -= VolFadeSpeed * float(now - LastInt) * 1.0e-3;
		}
		if(vol < 0)					vol = 0;
		if(MaxVol_SoundNoise < vol)	vol = MaxVol_SoundNoise;
		
		sound_Noise.setVolume(vol);
		
		LastInt = now;
	}
	
	void Transition(int NextState, int now)
	{
		/* */
		if((NextState < 0) || (NUM_STATE <= NextState)) return;
		
		/* */
		State = STATE(NextState);
		t_from_ms = now;
		
		switch(State){
			case STATE__SLEEP:
				sound_Noise.setVolume(0);
				break;
				
			case STATE__SLEEP_PLAYING:
				break;
				
			case STATE__RUN:
				duration = (int)ofRandom(60e+3, 70e+3);
				break;
				
			case STATE__CHANGING_CONTENTS:
				duration = (int)ofRandom(500, 600);
				break;
				
			case STATE__WAIT_STABLE:
				duration = 300;
				break;
				
			default:
				break;
		}
	}
	
	STATE get_State(){ return State; }
	
	int ms_To_Timeout(int now){ return duration - (now - t_from_ms); }
	
	bool IsTimeout(int now, ofVideoPlayer* video = NULL){
		if(!video)	return _IsTimeout(now);
		else		return _IsTimeout(video);
	}
	
	bool Is_SomeThingWrong(int now_ms, bool b_mov){
		if(!b_mov) return false;
		
		switch(State){
			case STATE__SLEEP:
			case STATE__SLEEP_PLAYING:
			case STATE__RUN:
			case STATE__CHANGING_CONTENTS:
				return false;
				
			case STATE__WAIT_STABLE:
				if(2000 < now_ms - t_from_ms){
					t_from_ms = now_ms;
					return true;
				}else{
					return false;
				}
				
			default:
				break;
		}
		
		return false;
	}
};