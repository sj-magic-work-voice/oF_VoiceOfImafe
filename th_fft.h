/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"

#include "sj_common.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class THREAD_FFT : public ofThread, private Noncopyable{
private:
	/****************************************
	****************************************/
	/********************
	********************/
	double Gain[AUDIO_BUF_SIZE];
	
	const int N;
	vector<float> fft_window;
	vector<double> sintbl;
	vector<int> bitrev;
	
	float LastInt;
	
	/****************************************
	function
	****************************************/
	/********************
	singleton
	********************/
	THREAD_FFT();
	~THREAD_FFT();
	THREAD_FFT(const THREAD_FFT&); // Copy constructor. no define.
	THREAD_FFT& operator=(const THREAD_FFT&); // コピー代入演算子. no define.
	
	/********************
	********************/
	void threadedFunction();
	
	int fft(double x[], double y[], int IsReverse = false);
	void make_bitrev(void);
	void make_sintbl(void);
	
	static int double_sort( const void * a , const void * b );
	void AudioSample_fft_LPF_saveToArray(const vector<float> &AudioSample, float dt);
	
public:
	/****************************************
	****************************************/
	/********************
	********************/
	static THREAD_FFT* getInstance(){
		static THREAD_FFT inst;
		return &inst;
	}
	
	void exit();
	void setup();
	void update();
	
	void update__Gain(const vector<float> &AudioSample);
	
	double getArrayVal(int id);
	double getArrayVal_x_DispGain(int id, float Gui_DispGain);
	
	void Log();
};



