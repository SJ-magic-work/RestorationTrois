/************************************************************
************************************************************/
#include "ofApp.h"

#include <time.h>

/* for dir search */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <dirent.h>
#include <string>

using namespace std;


/************************************************************
************************************************************/
using namespace ofxCv;
using namespace cv;


/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int _Cam_id, bool _b_flipCamera, bool _b_mov)
: Cam_id(_Cam_id)
, b_mov(_b_mov)
, c_TotalActivePixels(0)
, c_TotalActivePixels_Last(0)
, png_id(0)
, Contents_id(0)
, b_flipCamera(_b_flipCamera)
, VideoVol(0)
{
	/********************
	********************/
	srand((unsigned) time(NULL));
	
	/********************
	********************/
	font[FONT_S].load("font/RictyDiminished-Regular.ttf", 10, true, true, true);
	font[FONT_M].load("font/RictyDiminished-Regular.ttf", 12, true, true, true);
	font[FONT_L].load("font/RictyDiminished-Regular.ttf", 15, true, true, true);
	font[FONT_LL].load("font/RictyDiminished-Regular.ttf", 30, true, true, true);
}

/******************************
******************************/
ofApp::~ofApp()
{
}

/******************************
******************************/
void ofApp::setup(){

	/********************
	********************/
	ofSetWindowTitle("RESTORATION");
	
	ofSetWindowShape( WINDOW_WIDTH, WINDOW_HEIGHT );
	/*
	ofSetVerticalSync(false);
	ofSetFrameRate(0);
	/*/
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	//*/
	
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	setup_Gui();
	
	setup_Camera();
	if(!b_CamSearchFailed){
		/********************
		********************/
		shader_AddMask.load( "sj_shader/AddMask.vert", "sj_shader/AddMask.frag");
		shader_Mask.load( "sj_shader/mask.vert", "sj_shader/mask.frag");
		shader_Mask_x2.load( "sj_shader/mask_x2.vert", "sj_shader/mask_x2.frag");
		shader_Liquid.load( "sj_shader/Liquid.vert", "sj_shader/Liquid.frag");
		
		/********************
		********************/
		/* */
		img_Frame.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_COLOR);
		img_Frame_Gray.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_GRAYSCALE);
		img_LastFrame_Gray.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_GRAYSCALE);
		img_AbsDiff_BinGray.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_GRAYSCALE);
		img_BinGray_Cleaned.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_GRAYSCALE);
		img_Bin_RGB.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_COLOR);
		img_Bin_RGB_Blur.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_COLOR);
		
		/* */
		fbo_CamFrame.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, GL_RGBA);
		clear_fbo(fbo_CamFrame);
		
		fbo_Contents.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, GL_RGBA, 4);
		clear_fbo(fbo_Contents);
		
		fbo_Mask_S.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, GL_RGBA);
		Reset_FboMask();
		
		fbo_CurrentDiff_L.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, GL_RGBA);
		clear_fbo(fbo_CurrentDiff_L);
		
		fbo_PreOut.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, GL_RGBA);
		clear_fbo(fbo_PreOut);
		
		fbo_Liquid.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, GL_RGBA, 4);
		clear_fbo(fbo_Liquid);
		
		fbo_Out.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, GL_RGBA);
		clear_fbo(fbo_Out);
		
		/* */
		pix_Mask_S.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_COLOR);
		pix_Mask_L_Gray.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, OF_IMAGE_GRAYSCALE);
		img_Mask_S.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_COLOR);
		img_Mask_S_Gray.allocate(SIZE_S_WIDTH, SIZE_S_HEIGHT, OF_IMAGE_GRAYSCALE);
		img_Mask_L.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, OF_IMAGE_COLOR);
		img_Mask_L_Gray.allocate(SIZE_L_WIDTH, SIZE_L_HEIGHT, OF_IMAGE_GRAYSCALE);
		
		/* */
		ActivePoints_Of_Mask.resize(pix_Mask_S.getWidth() * pix_Mask_S.getHeight());
	
		/********************
		********************/
		myGlitch.setup(&fbo_Out);
		Clear_AllGlitch();
		
		/********************
		********************/
		if(b_mov){
			makeup_contents_list("../../../data/mov");
			Order_of_Contents.resize(Movies.size());
		}else{
			makeup_contents_list("../../../data/image");
			Order_of_Contents.resize(ArtPaints.size());
		}
		
		SJ_UTIL::FisherYates(Order_of_Contents);
		
		if(b_mov)		Start_Mov(get_Mov_of_id(Contents_id));
		else			Refresh_Fbo_Contents(get_Image_of_id(Contents_id));
		
		/********************
		********************/
		StateTop.setup(b_mov);
		StateNoise.setup(b_mov);
		StateRepair.setup(b_mov);
		
		/********************
		********************/
		SyphonServer.setName("RESTORATION");
	}
}

/******************************
******************************/
void ofApp::clear_fbo(ofFbo& fbo)
{
	fbo.begin();
	ofClear(0, 0, 0, 0);
	fbo.end();
}

/******************************
******************************/
void ofApp::inc_Contents_id()
{
	Contents_id = getNextId_of_Contents();
}

/******************************
******************************/
int ofApp::getNextId_of_Contents()
{
	int ret = Contents_id + 1;
	
	if(b_mov){
		if(Movies.size() <= ret) ret = 0;
	}else{
		if(ArtPaints.size() <= ret) ret = 0;
	}
	
	return ret;
}

/******************************
******************************/
int ofApp::getPrevId_of_Contents()
{
	int ret = Contents_id - 1;
	
	if(ret < 0){
		if(b_mov){
			ret = Movies.size() - 1;
		}else{
			ret = ArtPaints.size() - 1;
		}
	}
	
	return ret;
}

/******************************
******************************/
int ofApp::get_AheadId_of_Contents(int N)
{
	int ret = Contents_id + N;
	
	if(b_mov){
		ret = ret % Movies.size();
	}else{
		ret = ret % ArtPaints.size();
	}
	
	return ret;
}

/******************************
******************************/
ofImage& ofApp::get_Image_of_id(int _Contents_id)
{
	if(b_mov){ ERROR_MSG(); std::exit(1); }
	
	return ArtPaints[Order_of_Contents[_Contents_id]];
}

/******************************
******************************/
ofVideoPlayer* ofApp::get_Mov_of_id(int _Contents_id)
{
	if(!b_mov)	return NULL;
	else		return &Movies[Order_of_Contents[_Contents_id]];
}

/******************************
******************************/
void ofApp::Reset_FboMask()
{
	fbo_Mask_S.begin();
	ofSetColor(255, 255, 255, 255);
	ofDrawRectangle(0, 0, fbo_Mask_S.getWidth(), fbo_Mask_S.getHeight());
	
	fbo_Mask_S.end();
}

/******************************
******************************/
void ofApp::makeup_contents_list(const string dirname)
{
	/********************
	********************/
	DIR *pDir;
	struct dirent *pEnt;
	struct stat wStat;
	string wPathName;

	pDir = opendir( dirname.c_str() );
	if ( NULL == pDir ) { ERROR_MSG(); std::exit(1); }

	pEnt = readdir( pDir );
	while ( pEnt ) {
		// .と..は処理しない
		if ( strcmp( pEnt->d_name, "." ) && strcmp( pEnt->d_name, ".." ) ) {
		
			wPathName = dirname + "/" + pEnt->d_name;
			
			// ファイルの情報を取得
			if ( stat( wPathName.c_str(), &wStat ) ) {
				printf( "Failed to get stat %s \n", wPathName.c_str() );
				break;
			}
			
			if ( S_ISDIR( wStat.st_mode ) ) {
				// nothing.
			} else {
			
				vector<string> str = ofSplitString(pEnt->d_name, ".");
				
				if(b_mov){
					if( (str[str.size()-1] == "mp4") || (str[str.size()-1] == "mov") ){
						ofVideoPlayer _mov;
						
						_mov.load(wPathName);
						Movies.push_back(_mov);
						
						if(MAX_MOV_LOAD <= Movies.size()) break;
					}
				}else{
					if( (str[str.size()-1] == "png") || (str[str.size()-1] == "jpg") || (str[str.size()-1] == "jpeg") ){
						ofImage _image;
						_image.load(wPathName);
						ArtPaints.push_back(_image);
						
						if(MAX_IMG_LOAD <= ArtPaints.size()) break;
					}
				}
			}
		}
		
		pEnt = readdir( pDir ); // 次のファイルを検索する
	}

	closedir( pDir );
	
	/********************
	********************/
	if(b_mov){
		if(Movies.size() < 1)		{ ERROR_MSG();std::exit(1);}
		else						{ printf("> %d movies loaded\n", int(Movies.size())); fflush(stdout); }
	}else{
		if(ArtPaints.size() < 4)	{ ERROR_MSG();std::exit(1);}
		else						{ printf("> %d images loaded\n", int(ArtPaints.size())); fflush(stdout); }
	}
}

/******************************
******************************/
void ofApp::Start_Mov(ofVideoPlayer* video)
{
	if(video->isLoaded()){
		video->setLoopState(OF_LOOP_NONE);
		video->setSpeed(1);
		video->setVolume(VideoVol);
		video->setPaused(false);
		video->play();
		video->setPosition(0); // *.movは、"OF_LOOP_NONE"の時、明示的にsetPosition(0) しないと、戻らないようだ. : seekをplay の後に移動(2020.10.08)
	}
}

/******************************
******************************/
void ofApp::setup_Camera()
{
	/********************
	********************/
	printf("> setup camera\n");
	fflush(stdout);
	
	ofSetLogLevel(OF_LOG_VERBOSE);
    cam.setVerbose(true);
	
	vector< ofVideoDevice > Devices = cam.listDevices();// 上 2行がないと、List表示されない.
	
	/********************
	search for camera by device name.
	********************/
	if(Cam_id == -2){
		Cam_id = -1;
		
		int i;
		for(i = 0; i < Devices.size(); i++){
			if(Devices[i].deviceName == "HD Pro Webcam C920" ){
				Cam_id = i;
				break;
			}
		}
		
		if(i == Devices.size()){
			b_CamSearchFailed = true;
			t_CamSearchFailed = ofGetElapsedTimef();
			
			return;
		}
	}
	
	/********************
	********************/
	if(Cam_id == -1){
		std::exit(1);
	}else{
		if(Devices.size() <= Cam_id) { ERROR_MSG(); std::exit(1); }
		
		cam.setDeviceID(Cam_id);
		cam.initGrabber(SIZE_S_WIDTH, SIZE_S_HEIGHT);
		
		printf("> Cam size asked = (%d, %d), actual = (%d, %d)\n", int(SIZE_S_WIDTH), int(SIZE_S_HEIGHT), int(cam.getWidth()), int(cam.getHeight()));
		fflush(stdout);
	}
	
	return;
}

/******************************
description
	memoryを確保は、app start後にしないと、
	segmentation faultになってしまった。
******************************/
void ofApp::setup_Gui()
{
	/********************
	********************/
	Gui_Global = new GUI_GLOBAL;
	Gui_Global->setup("RESTORATION", "gui.xml", 1000, 10);
}

/******************************
******************************/
void ofApp::update(){
	/********************
	********************/
	if(b_CamSearchFailed){
		if(2.0 < ofGetElapsedTimef() - t_CamSearchFailed){
			ofExit(1);
			return;
		}else{
			return;
		}
	}

	/********************
	********************/
	int now = ofGetElapsedTimeMillis();
	
	if(b_mov){
		if(VideoVol != Gui_Global->VideoVol) { VideoVol = Gui_Global->VideoVol; get_Mov_of_id(Contents_id)->setVolume(VideoVol); }
	}
	
	ofSoundUpdate();
	
	/********************
	********************/
    cam.update();
    if(cam.isFrameNew())	{ update_img_OnCam(); }
	
	if(b_mov)	update_mov(get_Mov_of_id(Contents_id));
	
	update_img();
	
	/********************
	********************/
	StateChart_Top(now);
	StateChart_Noise(now);
	StateChart_Repair(now);
	
	/* */
	StateTop.update(now);
	StateNoise.update(now);
	
	/* */
	StateRepair.update(now);
	
	if(StateRepair.draw(now, fbo_Mask_S)){
		ofVec2f NextTarget = SelectNextTargetToRepair();
		StateRepair.SetNextTarget(NextTarget);
	}
	
	/********************
	********************/
	printf("%5d, %5d, %5d, %5.0f\r", StateTop.get_State(), StateNoise.get_State(), StateRepair.get_State(), ofGetFrameRate());
	fflush(stdout);
}

/******************************
******************************/
void ofApp::update_mov(ofVideoPlayer* video){
	if(!video) return;
	
	if(video->isLoaded()){
		video->update();
		if(video->isFrameNew()){
			Refresh_Fbo_Contents(video);
		}
	}
}

/******************************
******************************/
void ofApp::update_img_OnCam(){
	ofxCv::copy(cam, img_Frame);
	if(b_flipCamera) img_Frame.mirror(false, true);
	img_Frame.update();
	
	Copy_img_to_fbo(img_Frame, fbo_CamFrame);
	
	img_LastFrame_Gray = img_Frame_Gray;
	// img_LastFrame_Gray.update(); // drawしないので不要.
	
	convertColor(img_Frame, img_Frame_Gray, CV_RGB2GRAY);
	ofxCv::blur(img_Frame_Gray, ForceOdd((int)Gui_Global->BlurRadius_Frame));
	img_Frame_Gray.update();
}

/******************************
******************************/
void ofApp::Copy_img_to_fbo(ofImage& img, ofFbo& fbo){
	ofDisableAlphaBlending();
	
	fbo.begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255);
		
		img.draw(0, 0, fbo.getWidth(), fbo.getHeight());
	fbo.end();
}

/******************************
******************************/
void ofApp::update_img(){
	absdiff(img_Frame_Gray, img_LastFrame_Gray, img_AbsDiff_BinGray);
	threshold(img_AbsDiff_BinGray, Gui_Global->thresh_Diff_to_Bin);
	img_AbsDiff_BinGray.update();
	
	ofxCv::copy(img_AbsDiff_BinGray, img_BinGray_Cleaned);
	ofxCv::medianBlur(img_BinGray_Cleaned, ForceOdd((int)Gui_Global->MedianRadius));
	threshold(img_BinGray_Cleaned, Gui_Global->thresh_Medianed_to_Bin);
	img_BinGray_Cleaned.update();
	
	convertColor(img_BinGray_Cleaned, img_Bin_RGB, CV_GRAY2RGB);
	img_Bin_RGB.update();
	
#ifdef ADD_CURRENT_DIFF
	if((StateTop.get_State() == STATE_TOP::STATE__RUN) && (StateRepair.get_State() != STATE_REPAIR::STATE__STABLE)){
		ofxCv::copy(img_Bin_RGB, img_Bin_RGB_Blur);
		ofxCv::blur(img_Bin_RGB_Blur, ForceOdd((int)Gui_Global->BlurRadius_CurrentDiff_S));
		img_Bin_RGB_Blur.update();
		
		Copy_img_to_fbo(img_Bin_RGB_Blur, fbo_CurrentDiff_L);
	}else{
		clear_fbo(fbo_CurrentDiff_L);
	}
#endif
	
	if((StateTop.get_State() == STATE_TOP::STATE__RUN) && (StateRepair.get_State() == STATE_REPAIR::STATE__STABLE)) Add_DiffArea_To_MaskArea();
	fbo_Mask_S.readToPixels(pix_Mask_S);
	img_Mask_S.setFromPixels(pix_Mask_S);
	convertColor(img_Mask_S, img_Mask_S_Gray, CV_RGB2GRAY);
	
	/********************
	opencv:resize
		https://cvtech.cc/resize/
		
	OpenCVの膨張縮小って4近傍？8近傍?	
		http://micchysdiary.blogspot.com/2012/10/opencv48.html
	********************/
	cv::Mat srcMat = toCv(img_Mask_S_Gray);
	cv::Mat src_Cleaned_Mat;
	cv::Mat src_Eroded_Mat;
	cv::dilate(srcMat, src_Cleaned_Mat, Mat(), cv::Point(-1, -1), 2); // 白が拡大 : ゴミ取り
	cv::erode(src_Cleaned_Mat, src_Eroded_Mat, Mat(), cv::Point(-1, -1), (int)Gui_Global->ErodeSize); // 白に黒が侵食 : 拡大
	
	/* */
	toOf(src_Eroded_Mat, pix_Mask_S);
	img_Mask_S.setFromPixels(pix_Mask_S);
	img_Mask_S.update();
	
	c_TotalActivePixels_Last = c_TotalActivePixels;
	c_TotalActivePixels = Count_ActivePixel(pix_Mask_S, ofColor(255, 255, 255));
	
	/* */
	cv::Mat dstMat;// = toCv(img_Mask_L);
	// resize(src_Eroded_Mat, dstMat, dstMat.size(), 0, 0, INTER_LINEAR);
	resize(src_Eroded_Mat, dstMat, cv::Size(), img_Mask_L.getWidth()/src_Eroded_Mat.cols, img_Mask_L.getHeight()/src_Eroded_Mat.rows, INTER_LINEAR);
	
	toOf(dstMat, pix_Mask_L_Gray);
	img_Mask_L_Gray.setFromPixels(pix_Mask_L_Gray);
	
	ofxCv::blur(img_Mask_L_Gray, ForceOdd((int)Gui_Global->BlurRadius_MaskL));
	img_Mask_L_Gray.update();
	
	convertColor(img_Mask_L_Gray, img_Mask_L, CV_GRAY2RGB);
	img_Mask_L.update();
	
	/********************
	********************/
	Mask_x_Contents();
	
	LiquidEffect(fbo_PreOut, fbo_Liquid);
	
	drawFbo_Preout_to_Out(fbo_Liquid);
}

/******************************
******************************/
void ofApp::Mask_x_Contents()
{
	ofDisableAlphaBlending();
	
	fbo_PreOut.begin();
#ifdef ADD_CURRENT_DIFF
		if((StateTop.get_State() == STATE_TOP::STATE__RUN) && (StateRepair.get_State()  != STATE_REPAIR::STATE__STABLE)){
			shader_Mask_x2.begin();
			
				ofClear(0, 0, 0, 0);
				ofSetColor(255, 255, 255, 255);
				
				shader_Mask_x2.setUniformTexture( "Back", fbo_CamFrame.getTexture(), 1 );
				shader_Mask_x2.setUniformTexture( "mask_0", img_Mask_L.getTexture(), 2 );
				shader_Mask_x2.setUniformTexture( "mask_1", fbo_CurrentDiff_L.getTexture(), 3 );
				
				fbo_Contents.draw(0, 0, fbo_PreOut.getWidth(), fbo_PreOut.getHeight());
				
			shader_Mask_x2.end();
		}else{
			shader_Mask.begin();
			
				ofClear(0, 0, 0, 0);
				ofSetColor(255, 255, 255, 255);
				
				shader_Mask.setUniformTexture( "Back", fbo_CamFrame.getTexture(), 1 );
				shader_Mask.setUniformTexture( "mask", img_Mask_L.getTexture(), 2 );
				
				fbo_Contents.draw(0, 0, fbo_PreOut.getWidth(), fbo_PreOut.getHeight());
				
			shader_Mask.end();
		}
#else

		shader_Mask.begin();
		
			ofClear(0, 0, 0, 0);
			ofSetColor(255, 255, 255, 255);
			
			shader_Mask.setUniformTexture( "Back", fbo_CamFrame.getTexture(), 1 );
			shader_Mask.setUniformTexture( "mask", img_Mask_L.getTexture(), 2 );
			
			fbo_Contents.draw(0, 0, fbo_PreOut.getWidth(), fbo_PreOut.getHeight());
			
		shader_Mask.end();

#endif
	fbo_PreOut.end();
}

/******************************
******************************/
void ofApp::LiquidEffect(ofFbo& fbo_from, ofFbo& fbo_to)
{
	/********************
	ofEnableSmoothing() と OF_BLENDMODE_ADD は、一緒に動作しない.
	********************/
	/*
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	*/
	ofDisableAlphaBlending();
	ofEnableSmoothing();
	
	fbo_to.begin();
		ofSetColor(255);
		
		if(StateNoise.get__Amp_Liquid() == 0){
			fbo_from.draw(0, 0);
		}else{
			shader_Liquid.begin();
				// float time = (float)ofGetElapsedTimeMillis() / 1000;
				float time = ofGetElapsedTimef();
				shader_Liquid.setUniform1f( "time", time );
				shader_Liquid.setUniform1f( "Amp", StateNoise.get__Amp_Liquid() );
			
				ofSetColor( 255, 255, 255 );
				fbo_from.draw( 0, 0 );
			shader_Liquid.end();
		}
	fbo_to.end();
}

/******************************
******************************/
void ofApp::Refresh_Fbo_Contents(ofImage& img)
{
	ofDisableAlphaBlending();
	ofEnableSmoothing();
	
	fbo_Contents.begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		img.draw(0, 0, fbo_Contents.getWidth(), fbo_Contents.getHeight());
	fbo_Contents.end();
}

/******************************
******************************/
void ofApp::Refresh_Fbo_Contents(ofVideoPlayer* video)
{
	ofDisableAlphaBlending();
	ofEnableSmoothing();
	
	fbo_Contents.begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		if(video->isLoaded() && video->isPlaying()){
			video->draw(0, 0, fbo_Contents.getWidth(), fbo_Contents.getHeight());
		}else{
			ofSetColor(0, 0, 0, 255);
			ofDrawRectangle(0, 0, fbo_Contents.getWidth(), fbo_Contents.getHeight());
		}
	fbo_Contents.end();
}

/******************************
******************************/
void ofApp::Refresh_Fbo_Contents_onChangingContents(int now)
{
	/********************
	********************/
	int ret = DrawOffsetManager.update(now, fbo_Contents.getHeight());
	
	if(ret == 1){
		inc_Contents_id();
	}else if(ret == 2){
		// id : keep
		Refresh_Fbo_Contents(get_Image_of_id(Contents_id));
		
		Clear_AllGlitch();
		StateTop.Transition(STATE_TOP::STATE__WAIT_STABLE, now);
		
		return;
	}
	
	/********************
	********************/
	double ofs = DrawOffsetManager.get_ofs();
	
	ofDisableAlphaBlending();
	ofEnableSmoothing();
	
	fbo_Contents.begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		get_Image_of_id(Contents_id).draw(0, ofs, fbo_Contents.getWidth(), fbo_Contents.getHeight());
		get_Image_of_id(getNextId_of_Contents()).draw(0, ofs - fbo_Contents.getHeight(), fbo_Contents.getWidth(), fbo_Contents.getHeight());
	fbo_Contents.end();
}

/******************************
******************************/
void ofApp::StateChart_Top(int now){
	
	static int c_Retry_video = 0;
	
	switch(StateTop.get_State()){
		case STATE_TOP::STATE__SLEEP:
			if(!b_mov)											StateTop.Transition(STATE_TOP::STATE__SLEEP_PLAYING, now);
			else if(get_Mov_of_id(Contents_id)->isPlaying())	StateTop.Transition(STATE_TOP::STATE__SLEEP_PLAYING, now);
			break;
			
		case STATE_TOP::STATE__SLEEP_PLAYING:
			if(b_mov){
				if( get_Mov_of_id(Contents_id)->getIsMovieDone() || !(get_Mov_of_id(Contents_id)->isPlaying()) ){
					if(get_Mov_of_id(Contents_id)->isPlaying()){
						get_Mov_of_id(Contents_id)->stop();
						usleep(2000);
					}
					
					inc_Contents_id();
					Start_Mov(get_Mov_of_id(Contents_id));
	
					Refresh_Fbo_Contents(get_Mov_of_id(Contents_id));
					
					StateTop.Transition(STATE_TOP::STATE__SLEEP, now);
				}
			}
			
#ifdef AUTO_RUN_FROM_SLEEP
			if( StateTop.IsTimeout(now, get_Mov_of_id(Contents_id)) ){
				StateTop.Transition(STATE_TOP::STATE__RUN, ofGetElapsedTimeMillis());
				Clear_AllGlitch();
			}
#endif			
			break;
			
		case STATE_TOP::STATE__RUN:
			if( StateTop.IsTimeout(now, get_Mov_of_id(Contents_id)) ){
				StateTop.Transition(STATE_TOP::STATE__CHANGING_CONTENTS, now);
				
				Reset_FboMask();
				
				StateNoise.Transition(STATE_NOISE::STATE__CALM, now);
				StateRepair.Transition(STATE_REPAIR::STATE__STABLE, now);
				
				Clear_AllGlitch();
				if(b_mov) Random_Enable_myGlitch();
				
				if(!b_mov){
					int RandomNum = ofRandom(2, 4);
					int NextImageId = get_AheadId_of_Contents(RandomNum);
				
					DrawOffsetManager.start(now, RandomNum * fbo_Contents.getHeight(), RandomNum * fbo_Contents.getHeight() + 0, (RandomNum - 1) * fbo_Contents.getHeight());
				}
			}
			break;
			
		case STATE_TOP::STATE__CHANGING_CONTENTS:
			if(b_mov){
				if( get_Mov_of_id(Contents_id)->getIsMovieDone() || !get_Mov_of_id(Contents_id)->isPlaying() || StateTop.IsTimeout(now, get_Mov_of_id(Contents_id)) ){
					if(get_Mov_of_id(Contents_id)->isPlaying()) { get_Mov_of_id(Contents_id)->setPosition(0); get_Mov_of_id(Contents_id)->stop(); }
					
					inc_Contents_id();
					Start_Mov(get_Mov_of_id(Contents_id));
					Refresh_Fbo_Contents(get_Mov_of_id(Contents_id));
					
					Clear_AllGlitch();
					c_Retry_video = 0;
					StateTop.Transition(STATE_TOP::STATE__WAIT_STABLE, now);
				}
			}else{
				Refresh_Fbo_Contents_onChangingContents(now);
			}
			break;
			
		case STATE_TOP::STATE__WAIT_STABLE:
			if(StateTop.Is_SomeThingWrong(now, b_mov)){
				/* */
				if(3 < c_Retry_video){ ERROR_MSG(); std::exit(1); }
				
				/* */
				printf("\n> Something wrong @ STATE__WAIT_STABLE\n");
				fflush(stdout);
				
				if(get_Mov_of_id(Contents_id)->isPlaying()) { get_Mov_of_id(Contents_id)->stop(); }
				
				Start_Mov(get_Mov_of_id(Contents_id));
				
				c_Retry_video++;
				
			}else if( StateTop.IsTimeout(now, get_Mov_of_id(Contents_id)) ){
				StateTop.Transition(STATE_TOP::STATE__RUN, now);
			}
			break;
			
		default:
			break;
	}
}


/******************************
******************************/
void ofApp::StateChart_Noise(int now){

	if( !((StateTop.get_State() == STATE_TOP::STATE__RUN) && (StateRepair.get_State() == STATE_REPAIR::STATE__STABLE)) ) return;
	
	switch(StateNoise.get_State()){
		case STATE_NOISE::STATE__CALM:
			if(c_TotalActivePixels_Last < c_TotalActivePixels){
				StateNoise.Transition(STATE_NOISE::STATE__NOISE, now);
			}
			break;
			
		case STATE_NOISE::STATE__NOISE:
			if(c_TotalActivePixels <= c_TotalActivePixels_Last){
				StateNoise.Transition(STATE_NOISE::STATE__ECHO, now);
			}
			break;
			
		case STATE_NOISE::STATE__ECHO:
			if(StateNoise.IsTimeout(now)){
				StateNoise.Transition(STATE_NOISE::STATE__CALM, now);
			}else if(c_TotalActivePixels_Last < c_TotalActivePixels){
				StateNoise.Transition(STATE_NOISE::STATE__NOISE, now);
			}
			break;
			
		default:
			break;
	}
}

/******************************
******************************/
void ofApp::StateChart_Repair(int now){
	if(StateTop.get_State() != STATE_TOP::STATE__RUN)	return;
	
	switch(StateRepair.get_State()){
		case STATE_REPAIR::STATE__STABLE:
			if( (StateNoise.get_State() == STATE_NOISE::STATE__CALM) && (pix_Mask_S.getWidth() * pix_Mask_S.getHeight() * 0.7 < c_TotalActivePixels) ){
				StateRepair.Transition(STATE_REPAIR::STATE__WAIT_POWER_REPAIR, now);
			
			}else if( (StateNoise.get_State() == STATE_NOISE::STATE__CALM) && ( (0 < c_TotalActivePixels) && (c_TotalActivePixels < 40.0 * 40.0) ) ){
				StateRepair.Transition(STATE_REPAIR::STATE__SHORT_REPAIR, now);
			
			}else if( (StateNoise.get_State() == STATE_NOISE::STATE__CALM) && (0 < c_TotalActivePixels) ){
				ofVec2f NextTarget = SelectNextTargetToRepair();
				
				StateRepair.SetNextTarget(NextTarget);
				StateRepair.Transition(STATE_REPAIR::STATE__REPAIR, now);
				
				StateRepair.Set_PatchSize(c_TotalActivePixels);
			}
			break;
			
		case STATE_REPAIR::STATE__REPAIR:
			break;
			
		case STATE_REPAIR::STATE__RAPID:
			if(c_TotalActivePixels <= 0){
				StateRepair.Transition(STATE_REPAIR::STATE__BUFFER, now);
				Reset_FboMask();
			}else if(3 < StateRepair.get__c_Rapid()){
				StateRepair.Transition(STATE_REPAIR::STATE__BREAK, now);
			}
			
			break;
			
		case STATE_REPAIR::STATE__BREAK:
			if(StateRepair.IsTimeout(now)){
				if(0 < c_TotalActivePixels){
					ofVec2f NextTarget = SelectNextTargetToRepair();
					
					StateRepair.SetNextTarget(NextTarget);
					
					/* */
					int RandomNum = (int)( ((double)rand() / ((double)RAND_MAX + 1)) * 11 );

					if(RandomNum < 9)	StateRepair.Transition(STATE_REPAIR::STATE__REPAIR, now);
					else				StateRepair.Transition(STATE_REPAIR::STATE__RAPID, now);
					
				}else{
					StateRepair.Transition(STATE_REPAIR::STATE__BUFFER, now);
					Reset_FboMask();
				}
			}
			break;
			
		case STATE_REPAIR::STATE__BUFFER:
			if(StateRepair.IsTimeout(now)){
				StateRepair.Transition(STATE_REPAIR::STATE__STABLE, now);
			}
			break;
			
		case STATE_REPAIR::STATE__WAIT_POWER_REPAIR:
			if(StateRepair.IsTimeout(now)){
				StateRepair.Transition(STATE_REPAIR::STATE__POWER_REPAIR, now);
				Random_Enable_myGlitch();
			}
			break;
			
		case STATE_REPAIR::STATE__POWER_REPAIR:
			if(StateRepair.IsTimeout(now)){
				StateRepair.Transition(STATE_REPAIR::STATE__BUFFER, now);
				Clear_AllGlitch();
				Reset_FboMask();
			}
			break;
			
		case STATE_REPAIR::STATE__SHORT_REPAIR:
			// transition @ STATE_REPAIR::draw()
			break;
			
		default:
			break;
	}
}

/******************************
******************************/
ofVec2f ofApp::SizeConvert_StoL(ofVec2f from){
	return from * (float(SIZE_L_WIDTH) / SIZE_S_WIDTH);
}

/******************************
******************************/
ofVec2f ofApp::SelectNextTargetToRepair(){

	// pix_Mask_S = img_Mask_S.getPixels();
	
	int counter = 0;
	
	for(int _y = 0; _y < pix_Mask_S.getHeight(); _y++){
		for(int _x = 0; _x < pix_Mask_S.getWidth(); _x++){
			ofColor col = pix_Mask_S.getColor(_x, _y);
			// if( (col.r <= 0) && (col.g <= 0) && (col.b <= 0) ){
			if( col != ofColor(255, 255, 255) ){
				ActivePoints_Of_Mask[counter].x = _x;
				ActivePoints_Of_Mask[counter].y = _y;
				
				counter++;
			}
		}
	}
	
	if(counter == 0){
		return ofVec2f(0, 0);
	}else{
		int id = ofRandom(0, counter);
		if(counter <= id) id = 0; // 無い はずだけど、一応.
		
		return ActivePoints_Of_Mask[id];
	}
}

/******************************
******************************/
void ofApp::drawFbo_Preout_to_Out(ofFbo& fbo_pre)
{
	/********************
	********************/
	ofDisableAlphaBlending();
	
	fbo_Out.begin();
		ofClear(0, 0, 0, 0);
	
		ofSetColor(255, 255, 255, 255);
		
		fbo_pre.draw(0, 0, fbo_Out.getWidth(), fbo_Out.getHeight());
		
		if(StateRepair.IsDispCursor()){
			ofVec2f CursorPos = StateRepair.get_CursorPos();
			CursorPos = SizeConvert_StoL(CursorPos);
			
			/********************
			oFでは、加算合成とスムージングは同時に効かない。その為、加算合成の前にofDisableSmoothing()を記述。
				https://qiita.com/y_UM4/items/b03a66d932536b25b51a
			********************/
			ofEnableAlphaBlending();
			// ofEnableBlendMode(OF_BLENDMODE_ADD);
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			// ofDisableAlphaBlending();
			
			ofSetColor(255, 255, 0, 200);
			glPointSize(1.0);
			// glLineWidth(1);
			ofSetLineWidth(2);
			
			ofDrawLine(0, CursorPos.y, fbo_Out.getWidth(), CursorPos.y);
			ofDrawLine(CursorPos.x, 0, CursorPos.x, fbo_Out.getHeight());
		}
	fbo_Out.end();
	
	/********************
	********************/
	if( (StateRepair.get_State() != STATE_REPAIR::STATE__STABLE) && (StateRepair.get_State() != STATE_REPAIR::STATE__BUFFER) && (StateRepair.get_State() != STATE_REPAIR::STATE__SHORT_REPAIR) ){
		drawFbo_Scale(fbo_Out);
		drawFbo_String_Info_on_Repairing(fbo_Out);
	}
	
	if(StateRepair.get_State() == STATE_REPAIR::STATE__SHORT_REPAIR) { /* drawFbo_Cursor_On_ShortRepair(fbo_Out); */ drawFbo_Scale(fbo_Out); }
	
	if(StateRepair.get_State() == STATE_REPAIR::STATE__POWER_REPAIR) drawFbo_String_PowerRepairing(fbo_Out);
}

/******************************
******************************/
void ofApp::drawFbo_Cursor_On_ShortRepair(ofFbo& fbo)
{
	fbo.begin();
		float Cursor_x = StateRepair.get_CursorPos_ShortRepair(ofGetElapsedTimeMillis(), fbo_Out);
		
		ofEnableAlphaBlending();
		// ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		// ofDisableAlphaBlending();
		
		ofSetColor(255, 255, 0, 100);
		glPointSize(1.0);
		// glLineWidth(1);
		ofSetLineWidth(2);
		
		ofDrawLine(Cursor_x, 0, Cursor_x, fbo.getHeight());
		
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_String_Info_on_Repairing(ofFbo& fbo)
{
	/********************
	********************/
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	/********************
	********************/
	char buf[BUF_SIZE_S];
	const int X0		= 900;
	const int X1		= 1090;
	const int Y_INIT	= 70;
	const int Y_STEP	= 50;
	
	fbo.begin();
		/********************
		********************/
		ofSetColor(0, 0, 0, 100);
		ofDrawRectangle(fbo.getWidth() * 2/3, 0, fbo.getWidth() /3, fbo.getHeight());
		
		/********************
		********************/
		
		ofSetColor(255, 255, 255, 200);
		
		/* */
		int _y = Y_INIT;
		{
			sprintf(buf, "to be Repaired");
			font[FONT_L].drawString(buf, X0, _y);
			
			sprintf(buf, "%6d [pix]", c_TotalActivePixels);
			font[FONT_L].drawString(buf, X1, _y);
		}
		
		/* */
		{
			_y += Y_STEP;
			
			sprintf(buf, "to be Repaired");
			font[FONT_L].drawString(buf, X0, _y);
			
			sprintf(buf, "%6.2f [%%]", 100 * float(c_TotalActivePixels) / float(SIZE_S_WIDTH * SIZE_S_HEIGHT));
			font[FONT_L].drawString(buf, X1, _y);
		}
		
		/* */
		{
			_y += Y_STEP;
			
			ofVec2f CursorPos = StateRepair.get_CursorPos();
			CursorPos = SizeConvert_StoL(CursorPos);
			
			sprintf(buf, "Cursor Current");
			font[FONT_L].drawString(buf, X0, _y);
			
			
			sprintf(buf, "%6d, %6d", (int)CursorPos.x, (int)CursorPos.y);
			font[FONT_L].drawString(buf, X1, _y);
		}
		
		/* */
		{
			_y += Y_STEP;
			
			ofVec2f CursorPos = StateRepair.get_TargetCursorPos();
			CursorPos = SizeConvert_StoL(CursorPos);
			
			sprintf(buf, "Cursor Target");
			font[FONT_L].drawString(buf, X0, _y);
			
			
			sprintf(buf, "%6d, %6d", (int)CursorPos.x, (int)CursorPos.y);
			font[FONT_L].drawString(buf, X1, _y);
		}
		
		/* */
		{
			_y += Y_STEP;
			
			ofVec2f CursorPos = StateRepair.get_TargetCursorPos();
			CursorPos = SizeConvert_StoL(CursorPos);
			
			sprintf(buf, "Patch Size");
			font[FONT_L].drawString(buf, X0, _y);
			
			
			sprintf(buf, "%6d", (int)StateRepair.get_PatchSize());
			font[FONT_L].drawString(buf, X1, _y);
		}
		
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_String_PowerRepairing(ofFbo& fbo)
{
	ofDisableAlphaBlending();
	
	fbo.begin();
	ofSetColor(255, 0, 0, 255);
	
	char buf[BUF_SIZE_S];
	sprintf(buf, "REPAIRING NOW...");
	font[FONT_LL].drawString(buf, fbo.getWidth()/2 - font[FONT_LL].stringWidth(buf)/2, fbo.getHeight()/2);
	
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_Scale(ofFbo& fbo)
{
	/* */
	drawFbo_Scale_x(fbo, ofVec2f(0, 0), ofVec2f(1, 1));
	drawFbo_Scale_x(fbo, ofVec2f(0, fbo.getHeight()), ofVec2f(1, -1));
	
	drawFbo_ScaleLine_x(fbo);
	
	/* */
	drawFbo_Scale_y(fbo, ofVec2f(0, 0), ofVec2f(1, 1));
	drawFbo_Scale_y(fbo, ofVec2f(fbo.getWidth(), 0), ofVec2f(-1, 1));
	
	drawFbo_ScaleLine_y(fbo);
}

/******************************
******************************/
void ofApp::drawFbo_ScaleLine_x(ofFbo& fbo)
{
	// ofDisableAlphaBlending();
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	fbo.begin();
		
		ofSetColor(255, 255, 255, 80);
		glPointSize(1.0);
		// glLineWidth(1);
		ofSetLineWidth(1);
		
		/********************
		********************/
		const float step = 5;
		const float space = 20;
		
		int id;
		float _x;
		for(id = 0, _x = 0; _x < fbo.getWidth(); _x += step, id++){
			if(id % 10 == 0)	ofDrawLine(_x, space, _x, fbo.getHeight() - space);
		}
		
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_ScaleLine_y(ofFbo& fbo)
{
	// ofDisableAlphaBlending();
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	fbo.begin();
		
		ofSetColor(255, 255, 255, 80);
		glPointSize(1.0);
		// glLineWidth(1);
		ofSetLineWidth(1);
		
		/********************
		********************/
		const float step = 5;
		const float space = 20;
		
		int id;
		float _y;
		for(id = 0, _y = 0; _y < fbo.getHeight(); _y += step, id++){
			if(id % 10 == 0)		ofDrawLine(space, _y, fbo.getWidth() - space, _y);
		}
		
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_Scale_x(ofFbo& fbo, ofVec2f _pos, ofVec2f _scale)
{
	ofDisableAlphaBlending();
	
	fbo.begin();
		
		ofSetColor(255, 255, 255, 180);
		glPointSize(1.0);
		// glLineWidth(1);
		ofSetLineWidth(1);
		
		/********************
		********************/
		const float step = 5;
		
		ofPushMatrix();
			ofTranslate(_pos);
			ofScale(_scale.x, _scale.y);
			
			int id;
			float _x;
			for(id = 0, _x = 0; _x < fbo.getWidth(); _x += step, id++){
				if(id % 10 == 0)		ofDrawLine(_x, 0, _x, 15);
				else if(id % 5 == 0)	ofDrawLine(_x, 0, _x, 10);
				else					ofDrawLine(_x, 0, _x, 5);
			}
			
		ofPopMatrix();
		
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_Scale_y(ofFbo& fbo, ofVec2f _pos, ofVec2f _scale)
{
	ofDisableAlphaBlending();
	
	fbo.begin();
		
		ofSetColor(255, 255, 255, 180);
		glPointSize(1.0);
		// glLineWidth(1);
		ofSetLineWidth(1);
		
		/********************
		********************/
		const float step = 5;
		
		ofPushMatrix();
			ofTranslate(_pos);
			ofScale(_scale.x, _scale.y);
			
			int id;
			float _y;
			for(id = 0, _y = 0; _y < fbo.getHeight(); _y += step, id++){
				if(id % 10 == 0)		ofDrawLine(0, _y, 15, _y);
				else if(id % 5 == 0)	ofDrawLine(0, _y, 10, _y);
				else					ofDrawLine(0, _y, 5, _y);
			}
			
		ofPopMatrix();
		
	fbo.end();
}

/******************************
******************************/
void ofApp::Add_DiffArea_To_MaskArea()
{
	ofDisableAlphaBlending();
	
	fbo_Mask_S.begin();
	shader_AddMask.begin();
	
		// ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		shader_AddMask.setUniformTexture( "MaskAll", fbo_Mask_S.getTexture(), 1 );
		
		img_Bin_RGB.draw(0, 0, fbo_Mask_S.getWidth(), fbo_Mask_S.getHeight());
		
	shader_AddMask.end();
	fbo_Mask_S.end();
}

/******************************
******************************/
int ofApp::Count_ActivePixel(const ofPixels& pix, ofColor InActive)
{
	int counter = 0;
	
	for(int _y = 0; _y < pix.getHeight(); _y++){
		for(int _x = 0; _x < pix.getWidth(); _x++){
			ofColor col = pix.getColor(_x, _y);
			if(col != InActive) counter++;
		}
	}
	
	return counter;
}

/******************************
******************************/
int ofApp::ForceOdd(int val){
	if(val <= 0)			return val;
	else if(val % 2 == 0)	return val - 1;
	else					return val;
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	if(b_CamSearchFailed){
		ofBackground(0);
		ofSetColor(255, 0, 0, 255);
		
		char buf[BUF_SIZE_S];
		sprintf(buf, "USB Camera not Exsist");
		font[FONT_L].drawString(buf, ofGetWidth()/2 - font[FONT_L].stringWidth(buf)/2, ofGetHeight()/2);
		
		return;
	}
	
	/********************
	********************/
	/*
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	*/
	ofDisableAlphaBlending();
	
	/********************
	********************/
	// ofClear(0, 0, 0, 0);
	ofBackground(50);
	ofSetColor(255, 255, 255, 255);
	
	/********************
	********************/
	myGlitch.generateFx(); // Apply effects
	
	/********************
	********************/
	draw_ProcessLine();
	
	draw_ProcessedImages();
	
	/********************
	********************/
	ofTexture tex = fbo_Out.getTextureReference();
	SyphonServer.publishTexture(&tex);
	
	/********************
	********************/
	Gui_Global->gui.draw();
}

/******************************
******************************/
void ofApp::draw_ProcessLine(){
	/********************
	********************/
	ofSetColor(250);
	glPointSize(1.0);
	// glLineWidth(1);
	ofSetLineWidth(1);
	
	
	ofDrawLine(240, 135, 1040, 135);
	ofDrawLine(240, 360, 1040, 360);
	ofDrawLine(240, 585, 1040, 585);
	
	ofDrawLine(1040, 135, 1040, 360);
	ofDrawLine(240, 360, 240, 585);
}

/******************************
******************************/
void ofApp::draw_ProcessedImages(){
	/********************
	********************/
	ofSetColor(255, 255, 255, 255);
	
	/********************
	********************/
	/* */
	img_Frame_Gray.draw(80, 45, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	img_AbsDiff_BinGray.draw(480, 45, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	
	// img_BinGray_Cleaned.draw(880, 45, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	fbo_CurrentDiff_L.draw(880, 45, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	
	/* */
	fbo_Mask_S.draw(880, 270, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	
	img_Mask_S.draw(480, 270, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	char buf[BUF_SIZE_S];
	sprintf(buf, "%6d / %6d", c_TotalActivePixels, int(img_Mask_S.getWidth() * img_Mask_S.getHeight()));
	font[FONT_M].drawString(buf, 800 - font[FONT_M].stringWidth(buf), 450 + font[FONT_M].stringHeight(buf) * 1.1);
	
	img_Mask_L.draw(80, 270, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	
	/* */
	fbo_Contents.draw(80, 495, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	fbo_PreOut.draw(480, 495, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	
	fbo_Out.draw(880, 495, SIZE_S_WIDTH, SIZE_S_HEIGHT);
	if(StateRepair.get_State() != STATE_REPAIR::STATE__STABLE){
		sprintf(buf, "%6d", int(StateRepair.get_PatchSize()));
		font[FONT_M].drawString(buf, 1200 - font[FONT_M].stringWidth(buf), 675 + font[FONT_M].stringHeight(buf) * 1.1);
	}
}

/******************************
******************************/
void ofApp::Random_Enable_myGlitch(){
	SJ_UTIL::FisherYates(Glitch_Ids, NUM_GLITCH_TYPES);
	NumGlitch_Enable = int(ofRandom(1, MAX_NUM_GLITCHS_ONE_TIME));
	for(int i = 0; i < NumGlitch_Enable; i++) { Set_myGlitch(Glitch_Ids[i], true); }
}

/******************************
******************************/
void ofApp::Set_myGlitch(int key, bool b_switch)
{
	if (key == 0)	myGlitch.setFx(OFXPOSTGLITCH_INVERT			, b_switch);
	if (key == 1)	myGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE	, b_switch);
	if (key == 2)	myGlitch.setFx(OFXPOSTGLITCH_GLOW			, b_switch);
	if (key == 3)	myGlitch.setFx(OFXPOSTGLITCH_SHAKER			, b_switch);
	if (key == 4)	myGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER		, b_switch);
	if (key == 5)	myGlitch.setFx(OFXPOSTGLITCH_TWIST			, b_switch);
	if (key == 6)	myGlitch.setFx(OFXPOSTGLITCH_OUTLINE		, b_switch);
	if (key == 7)	myGlitch.setFx(OFXPOSTGLITCH_NOISE			, b_switch);
	if (key == 8)	myGlitch.setFx(OFXPOSTGLITCH_SLITSCAN		, b_switch);
	if (key == 9)	myGlitch.setFx(OFXPOSTGLITCH_SWELL			, b_switch);
	if (key == 10)	myGlitch.setFx(OFXPOSTGLITCH_CR_HIGHCONTRAST, b_switch);
	if (key == 11)	myGlitch.setFx(OFXPOSTGLITCH_CR_BLUERAISE	, b_switch);
	if (key == 12)	myGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE	, b_switch);
	if (key == 13)	myGlitch.setFx(OFXPOSTGLITCH_CR_GREENRAISE	, b_switch);
	if (key == 14)	myGlitch.setFx(OFXPOSTGLITCH_CR_BLUEINVERT	, b_switch);
	if (key == 15)	myGlitch.setFx(OFXPOSTGLITCH_CR_REDINVERT	, b_switch);
	if (key == 16)	myGlitch.setFx(OFXPOSTGLITCH_CR_GREENINVERT	, b_switch);
}

/******************************
******************************/
void ofApp::Clear_AllGlitch()
{
	for(int i = 0; i < NUM_GLITCH_TYPES; i++){
		Set_myGlitch(i, false);
	}
}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case 'r':
			if(StateTop.get_State() == STATE_TOP::STATE__SLEEP_PLAYING){
				StateTop.Transition(STATE_TOP::STATE__RUN, ofGetElapsedTimeMillis());
				Clear_AllGlitch();
			}
			break;
			
		case 's':
			if(StateTop.get_State() == STATE_TOP::STATE__RUN){
				int now = ofGetElapsedTimeMillis();
				
				StateTop.Transition(STATE_TOP::STATE__SLEEP, now);
				Reset_FboMask();
				StateNoise.Transition(STATE_NOISE::STATE__CALM, now);
				StateRepair.Transition(STATE_REPAIR::STATE__STABLE, now);
				Clear_AllGlitch();
			}
			break;
		
		case 'v':
			printf("\n----------\n");
			for(int i = 0; i < Movies.size(); i++){
				printf("[%d] %d, %d, %d : %5.3f\n", i, Movies[i].isLoaded(), Movies[i].isPlaying(), Movies[i].getIsMovieDone(), Movies[i].getPosition());
			}
			fflush(stdout);
			break;
			
		case ' ':
			{
				char buf[512];
				
				sprintf(buf, "image_%d.png", png_id);
				ofSaveScreen(buf);
				// ofSaveFrame();
				printf("> %s saved\n", buf);
				
				png_id++;
			}
			
			break;
			
		default:
			break;
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
