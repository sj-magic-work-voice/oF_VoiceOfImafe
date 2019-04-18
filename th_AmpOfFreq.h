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
class AMP_OF_FREQ : private Noncopyable{
private:
	/****************************************
	****************************************/
	enum{
		NUM_HARMONIES = 3,
		
	};
	double Freq[NUM_HARMONIES];
	double Amp;
	double k_HighKeep[NUM_HARMONIES];
	double k_HighDown[NUM_HARMONIES];
	
	const double k_smooth; // プツ音 防止
	
public:
	/****************************************
	****************************************/
	AMP_OF_FREQ()
	: Amp(0)
	// , k_smooth(0.01)
	, k_smooth(0.1)
	{
		k_HighKeep[0] = 1.0;
		k_HighKeep[1] = 0.5 * k_HighKeep[0];
		k_HighKeep[2] = 0.33 * k_HighKeep[0];
		
		k_HighDown[0] = 1.0;
		k_HighDown[1] = 0.5 * k_HighDown[0];
		k_HighDown[2] = 0.33 * k_HighDown[0];
	}
	
	~AMP_OF_FREQ()
	{
	}
	
	void set_Freq(double _freq)
	{
		/********************
		********************/
		for(int i = 0; i < NUM_HARMONIES; i++){
			Freq[i] = _freq * (i + 1);
		}
		
		/********************
		********************/
		double g0 = 1.0;
		double g1 = 0.01;
		double f0 = 27.5;
		double f1 = 20000;
		
		double tan = (g1 - g0) / (log10(f1) - log10(f0));
		k_HighDown[0] = tan * (log10(_freq) - log10(f0)) + g0;
		
		/********************
		********************/
		k_HighDown[1] = 0.5 * k_HighDown[0];
		k_HighDown[2] = 0.33 * k_HighDown[0];
	}
	
	void set_Amp(double _Amp) {
		Amp = k_smooth * _Amp + (1 - k_smooth) * Amp;
	}
	
	void clear_Amp(){
		Amp = 0;
	}
	
	double get_Freq(int HarmonyId) { if(NUM_HARMONIES <= HarmonyId) HarmonyId = 0; return Freq[HarmonyId]; }
	
	double get_Amp(int HarmonyId, bool b_HighDown) {
		if(NUM_HARMONIES <= HarmonyId) HarmonyId = 0;
		
		if(b_HighDown)	return Amp * k_HighDown[HarmonyId];
		else			return Amp * k_HighKeep[HarmonyId];
	}
};


/**************************************************
**************************************************/
class THREAD_AMP_OF_FREQ : public ofThread, private Noncopyable{
private:
	/****************************************
	****************************************/
	/********************
	********************/
	AMP_OF_FREQ Array_AmpOfFreq[FBO_CAL_HEIGHT];
	int NumHarmony;
	
	/****************************************
	function
	****************************************/
	/********************
	singleton
	********************/
	THREAD_AMP_OF_FREQ();
	~THREAD_AMP_OF_FREQ();
	THREAD_AMP_OF_FREQ(const THREAD_AMP_OF_FREQ&); // Copy constructor. no define.
	THREAD_AMP_OF_FREQ& operator=(const THREAD_AMP_OF_FREQ&); // コピー代入演算子. no define.
	
	/********************
	********************/
	void threadedFunction();
	
public:
	/****************************************
	****************************************/
	/********************
	********************/
	static THREAD_AMP_OF_FREQ* getInstance(){
		static THREAD_AMP_OF_FREQ inst;
		return &inst;
	}
	
	void exit();
	void setup();
	void update_NumHarmony(int _NumHarmony);
	int get_NumHarmony();
	void update_Amp(int Nth, double value);
	double get_Amp(int Nth, int HarmonyId, bool b_HighDown = false);
	double get_Freq(int Nth, int HarmonyId);
	void clear_Amp();
};



