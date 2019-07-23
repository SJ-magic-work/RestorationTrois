/************************************************************
compile SW
	main.cpp
		#define BOOT_MODE BOOT_MODE__KAISAIYU

	sj_common.h
		#define AUTO_RUN_FROM_SLEEP
		#define ADD_CURRENT_DIFF
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "stdio.h"

#include "ofMain.h"
#include "ofxGui.h"

/************************************************************
************************************************************/
// #define AUTO_RUN_FROM_SLEEP
#define ADD_CURRENT_DIFF

/************************************************************
************************************************************/
#define ERROR_MSG(); printf("Error in %s:%d\n", __FILE__, __LINE__);

/************************************************************
************************************************************/
enum{
	WINDOW_WIDTH	= 1280,	// 切れの良い解像度でないと、ofSaveScreen()での画面保存が上手く行かなかった(真っ暗な画面が保存されるだけ).
	WINDOW_HEIGHT	= 720,
};

enum{
	SIZE_S_WIDTH	= 320,
	SIZE_S_HEIGHT	= 180,
	
	SIZE_L_WIDTH	= 1280,
	SIZE_L_HEIGHT	= 720,
};

enum{
	BUF_SIZE_S = 500,
	BUF_SIZE_M = 1000,
	BUF_SIZE_L = 6000,
};


/************************************************************
************************************************************/

/**************************************************
Derivation
	class MyClass : private Noncopyable {
	private:
	public:
	};
**************************************************/
class Noncopyable{
protected:
	Noncopyable() {}
	~Noncopyable() {}

private:
	void operator =(const Noncopyable& src);
	Noncopyable(const Noncopyable& src);
};


/**************************************************
**************************************************/
class GUI_GLOBAL{
private:
	/****************************************
	****************************************/
	
public:
	/****************************************
	****************************************/
	void setup(string GuiName, string FileName = "gui.xml", float x = 10, float y = 10);
	
	ofxGuiGroup Group_ImageProcess;
		ofxFloatSlider BlurRadius_Frame;
		ofxFloatSlider thresh_Diff_to_Bin;
		ofxFloatSlider MedianRadius;
		ofxFloatSlider thresh_Medianed_to_Bin;
		ofxFloatSlider ErodeSize;
		ofxFloatSlider BlurRadius_CurrentDiff_S;
		ofxFloatSlider BlurRadius_MaskL;
		
	ofxGuiGroup Group_Sound;
		ofxFloatSlider dt_Noise_Rise;
		ofxFloatSlider dt_Noise_Fall;
		
	ofxGuiGroup Group_Liquid;
		ofxFloatSlider dt_LiquidAmp_Rise;
		ofxFloatSlider dt_LiquidAmp_Fall;
		
	ofxGuiGroup Group_Volume;
		ofxFloatSlider VideoVol;
		
	/****************************************
	****************************************/
	ofxPanel gui;
};

/************************************************************
************************************************************/
double LPF(double LastVal, double CurrentVal, double Alpha_dt, double dt);
double LPF(double LastVal, double CurrentVal, double Alpha);
double sj_max(double a, double b);

/************************************************************
************************************************************/
extern GUI_GLOBAL* Gui_Global;

extern FILE* fp_Log;

extern int GPIO_0;
extern int GPIO_1;


/************************************************************
************************************************************/

