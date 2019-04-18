/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "stdio.h"

#include "ofMain.h"
#include "ofxGui.h"

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
	FBO_WIDTH	= 640,
	FBO_HEIGHT	= 360,
};

enum{
	FBO_CAL_WIDTH	= 160,
	FBO_CAL_HEIGHT	= 90,
};

enum{
	BUF_SIZE_S = 500,
	BUF_SIZE_M = 1000,
	BUF_SIZE_L = 6000,
};

enum{
	// AUDIO_BUF_SIZE = 512,
	AUDIO_BUF_SIZE = 2048,
	
	AUDIO_BUFFERS = 2,
	AUDIO_SAMPLERATE = 44100,
};

enum{
	GRAPH_BAR_WIDTH__FFT_GAIN = 1,
	GRAPH_BAR_SPACE__FFT_GAIN = 1,
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
	
	ofxGuiGroup Group_Sound;
		ofxFloatSlider vol;
		ofxFloatSlider NumHarmony;
		ofxFloatSlider CursorSpeed;
		ofxFloatSlider d_ROTATION;
		ofxFloatSlider d_PAUSE;
		ofxToggle b_HighDown;
		ofxToggle b_ImageChange;
		ofxToggle b_PALINDROME;
	
	ofxGuiGroup Group_FFT;
		ofxFloatSlider LPFAlpha_dt__FFTGain;
		ofxFloatSlider Val_DispMax__FFTGain;
	
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
extern FILE* fp_Log_main;
extern FILE* fp_Log_Audio;
extern FILE* fp_Log_fft;

extern int GPIO_0;
extern int GPIO_1;


/************************************************************
************************************************************/

