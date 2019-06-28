/************************************************************
■ofxCv
	https://github.com/kylemcdonald/ofxCv
	
	note
		Your linker will also need to know where the OpenCv headers are. In XCode this means modifying one line in Project.xconfig:
			HEADER_SEARCH_PATHS = $(OF_CORE_HEADERS) "../../../addons/ofxOpenCv/libs/opencv/include/" "../../../addons/ofxCv/libs/ofxCv/include/"
			
■openFrameworksのAddon、ofxCvの導入メモ
	https://qiita.com/nenjiru/items/50325fabe4c3032da270
	
	contents
		導入時に、一手間 必要.
		
■Kill camera process
	> sudo killall VDCAssistant
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxPostGlitch.h"
#include "ofxSyphon.h"

#include "sj_common.h"
#include "util.h"

#include "StateTop.h"
#include "StateNoise.h"
#include "StateRepair.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		FONT_S,
		FONT_M,
		FONT_L,
		FONT_LL,
		
		NUM_FONTSIZE,
	};
	
	enum{
		NUM_GLITCH_TYPES = 17,
		MAX_NUM_GLITCHS_ONE_TIME = 5,
	};
	
	enum{
		MAX_MOV_LOAD = 5,
	};
	
	/****************************************
	****************************************/
	ofTrueTypeFont font[NUM_FONTSIZE];
	int png_id;
	
	bool b_mov;
	
	ofVideoGrabber cam;
	int Cam_id;
	
	STATE_TOP StateTop;
	STATE_NOISE StateNoise;
	STATE_REPAIR StateRepair;
	
	int c_TotalActivePixels;
	int c_TotalActivePixels_Last;
	
    ofImage img_Frame;
    ofImage img_Frame_Gray;
    ofImage img_LastFrame_Gray;
    ofImage img_AbsDiff_BinGray;
    ofImage img_BinGray_Cleaned;
    ofImage img_Bin_RGB;
	
    ofFbo fbo_CamFrame;
    ofFbo fbo_Contents;
	ofFbo fbo_Mask_S;
	ofFbo fbo_PreOut;
	ofFbo fbo_Liquid;
	ofFbo fbo_Out;
	
	ofPixels pix_Mask_S;
	ofPixels pix_Mask_L_Gray;
	ofImage img_Mask_S;
	ofImage img_Mask_S_Gray;
	ofImage img_Mask_L;
	ofImage img_Mask_L_Gray;
	
	/********************
	********************/
	ofShader shader_AddMask;
	ofShader shader_Mask;
	ofShader shader_Liquid;
	
	/********************
	********************/
	vector<ofVec2f> ActivePoints_Of_Mask;
	
	/********************
	********************/
	int Contents_id;
	vector<int> Order_of_Contents;
	vector<ofImage> ArtPaints;
	vector<ofVideoPlayer> Movies;
	
	float VideoVol;
	
	/********************
	********************/
	ofxPostGlitch	myGlitch;
	int Glitch_Ids[NUM_GLITCH_TYPES];
	int NumGlitch_Enable;
	
	/********************
	********************/
	ofxSyphonServer SyphonServer;
	
	bool b_flipCamera;
	
	/****************************************
	****************************************/
	void clear_fbo(ofFbo& fbo);
	void inc_Contents_id();
	int getNextId_of_Contents();
	ofImage& get_Active_Image();
	ofVideoPlayer* get_Active_Mov();
	void Reset_FboMask();
	void Refresh_Fbo_Contents(ofImage& img);
	void Refresh_Fbo_Contents(ofVideoPlayer* video);
	void makeup_contents_list(const string dirname);
	void Start_Mov(ofVideoPlayer* video);
	void setup_Camera();
	void setup_Gui();
	void update_mov(ofVideoPlayer* video);
	void update_img_OnCam();
	void Copy_CamFrame_to_fbo();
	void update_img();
	void Mask_x_Contents();
	void LiquidEffect(ofFbo& fbo_from, ofFbo& fbo_to);
	void StateChart_Top(int now);
	void StateChart_Noise(int now);
	void StateChart_Repair(int now);
	ofVec2f SizeConvert_StoL(ofVec2f from);
	ofVec2f SelectNextTargetToRepair();
	void drawFbo_Preout_to_Out(ofFbo& fbo_pre);
	void drawFbo_String_Info_on_Repairing(ofFbo& fbo);
	void drawFbo_String_PowerRepairing(ofFbo& fbo);
	void drawFbo_Scale(ofFbo& fbo);
	void drawFbo_ScaleLine_x(ofFbo& fbo);
	void drawFbo_ScaleLine_y(ofFbo& fbo);
	void drawFbo_Scale_x(ofFbo& fbo, ofVec2f _pos, ofVec2f _scale);
	void drawFbo_Scale_y(ofFbo& fbo, ofVec2f _pos, ofVec2f _scale);
	void Add_DiffArea_To_MaskArea();
	int Count_ActivePixel(const ofPixels& pix, ofColor InActive);
	int ForceOdd(int val);
	void draw_ProcessLine();
	void draw_ProcessedImages();
	void Random_Enable_myGlitch();
	void Set_myGlitch(int key, bool b_switch);
	void Clear_AllGlitch();
	
public:
	/****************************************
	****************************************/
	ofApp(int _Cam_id, bool _b_flipCamera, bool _b_mov);
	~ofApp();

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
};
