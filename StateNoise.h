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
class STATE_NOISE{
public:
	enum STATE{
		STATE__CALM,
		STATE__NOISE,
		STATE__ECHO,
		
		NUM_STATE,
	};
	
private:	
	ofSoundPlayer sound_Noise;
	const float MaxVol_SoundNoise;
	const float MinVol_SoundNoise;
	
	float Amp_Liquid;
	const float Amp_Liquid__Max;
	const float Amp_Liquid__Min;
	
	STATE State;
	int t_from_ms;
	const int duration;
	int LastINT;
	
	void update_vol(int now)
	{
		float d_vol;
		
		float vol = sound_Noise.getVolume();
		switch(State){
			case STATE__CALM:
			case STATE__ECHO:
				d_vol = ( (MaxVol_SoundNoise - MinVol_SoundNoise) / float(Gui_Global->dt_Noise_Fall) ) * (now - LastINT);
				vol -= d_vol;
				if(vol < MinVol_SoundNoise) vol = MinVol_SoundNoise;
				break;
				
			case STATE__NOISE:
				d_vol = ( (MaxVol_SoundNoise - MinVol_SoundNoise) / float(Gui_Global->dt_Noise_Rise) ) * (now - LastINT);
				vol += d_vol;
				if(MaxVol_SoundNoise < vol) vol = MaxVol_SoundNoise;
				break;
				
			default:
				break;
		}
		sound_Noise.setVolume(vol);
	}
	
	void update_LiquidAmp(int now)
	{
		float d_Amp;
		
		switch(State){
			case STATE__CALM:
			case STATE__ECHO:
				d_Amp = ( (Amp_Liquid__Max - Amp_Liquid__Min) / float(Gui_Global->dt_LiquidAmp_Fall) ) * (now - LastINT);
				Amp_Liquid -= d_Amp;
				if(Amp_Liquid < Amp_Liquid__Min) Amp_Liquid = Amp_Liquid__Min;
				break;
				
			case STATE__NOISE:
				d_Amp = ( (Amp_Liquid__Max - Amp_Liquid__Min) / float(Gui_Global->dt_LiquidAmp_Rise) ) * (now - LastINT);
				Amp_Liquid += d_Amp;
				if(Amp_Liquid__Max < Amp_Liquid) Amp_Liquid = Amp_Liquid__Max;
				break;
				
			default:
				break;
		}
	}
	
public:
	STATE_NOISE()
	: State(STATE__CALM)
	, duration(1000)
	, MaxVol_SoundNoise(0.8)
	, MinVol_SoundNoise(0.0)
	, LastINT(0)
	, Amp_Liquid__Max(30.0)
	, Amp_Liquid__Min(0.0)
	, Amp_Liquid(0.0)
	{
	}
	
	void setup(bool b_mov){
		if(b_mov)	SJ_UTIL::setup_sound(sound_Noise, "sound/Noise/water.wav", true, 0.0);
		else		SJ_UTIL::setup_sound(sound_Noise, "sound/Noise/water.wav", true, 0.0);
		
		sound_Noise.play();
	}
	
	void update(int now)
	{
		update_vol(now);
		update_LiquidAmp(now);
		
		LastINT = now;
	}
	
	void Transition(int NextState, int now)
	{
		if((0 <= NextState) && (NextState < NUM_STATE)){
			State = STATE(NextState);
			t_from_ms = now;
		}
	}
	
	STATE get_State(){ return State; }
	
	float get__Amp_Liquid(){ return Amp_Liquid; }
	
	bool IsTimeout(int now){
		if(State == STATE__ECHO){
			if(duration < now - t_from_ms)	return true;
			else							return false;
		}else{
			return false;
		}
	}
};
	