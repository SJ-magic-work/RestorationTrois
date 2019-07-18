#include "ofMain.h"
#include "ofApp.h"

#define BOOT_MODE__DEFAULT	0
#define BOOT_MODE__KAISAIYU	1
#define BOOT_MODE__SUNAD 	2
	#define BOOT_MODE BOOT_MODE__KAISAIYU

//========================================================================
int main( int argc, char** argv ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	/********************
	********************/
#if (BOOT_MODE == BOOT_MODE__DEFAULT)
	int Cam_id = -1;
	bool b_flipCamera = true;
	bool b_mov = false;
#elif(BOOT_MODE == BOOT_MODE__KAISAIYU)
	int Cam_id = -2;
	bool b_flipCamera = true;
	bool b_mov = false;
#elif(BOOT_MODE == BOOT_MODE__SUNAD)
	int Cam_id = -2;
	bool b_flipCamera = true;
	bool b_mov = true;
#endif

	/********************
	********************/
	printf("---------------------------------\n");
	printf("> parameters\n");
	printf("\t-c:camera(-1)\n");
	printf("\t-f:flip camera(true)\n");
	printf("\t-m:use movie...or still(false)\n");
	printf("---------------------------------\n");
	
	for(int i = 1; i < argc; i++){
		if( strcmp(argv[i], "-c") == 0 ){
			if(i+1 < argc) Cam_id = atoi(argv[i+1]);
		}else if( strcmp(argv[i], "-f") == 0 ){
			if(i+1 < argc) b_flipCamera = atoi(argv[i+1]);
		}else if( strcmp(argv[i], "-m") == 0 ){
			if(i+1 < argc) b_mov = atoi(argv[i+1]);
		}
	}
	
	ofRunApp(new ofApp(Cam_id, b_flipCamera, b_mov));
}
