/************************************************************
************************************************************/
#include "th_AmpOfFreq.h"
#include "stdlib.h"

/************************************************************
************************************************************/

/******************************
******************************/
THREAD_AMP_OF_FREQ::THREAD_AMP_OF_FREQ()
: NumHarmony(1)
{
	double Freq_H = 20000;
	double Freq_L = 27.5;
	double a = (double)FBO_CAL_HEIGHT / (log10(Freq_H) - log10(Freq_L));
	double b = log10(Freq_L) * a;
	
	for(int _y = 0; _y < FBO_CAL_HEIGHT; _y++){
		int y = (FBO_CAL_HEIGHT - 1) - _y;
		
		double f = pow(10, (y + b) / a);
		Array_AmpOfFreq[_y].set_Freq(f);
	}
}

/******************************
******************************/
THREAD_AMP_OF_FREQ::~THREAD_AMP_OF_FREQ()
{
}

/******************************
******************************/
void THREAD_AMP_OF_FREQ::threadedFunction()
{
	while(isThreadRunning()) {
		lock();
		
		unlock();
		
		
		sleep(10);
	}
}

/******************************
******************************/
void THREAD_AMP_OF_FREQ::exit()
{
	this->lock();
	
	this->unlock();
}

/******************************
******************************/
void THREAD_AMP_OF_FREQ::setup()
{
	this->lock();
	NumHarmony = Gui_Global->NumHarmony;
	this->unlock();
}

/******************************
******************************/
void THREAD_AMP_OF_FREQ::clear_Amp()
{
	this->lock();
	
	for(int i = 0; i < FBO_CAL_HEIGHT; i++){
		Array_AmpOfFreq[i].clear_Amp();
	}
	
	this->unlock();
}

/******************************
******************************/
void THREAD_AMP_OF_FREQ::update_NumHarmony(int _NumHarmony)
{
	this->lock();
	NumHarmony = _NumHarmony;
	this->unlock();
}

/******************************
******************************/
int THREAD_AMP_OF_FREQ::get_NumHarmony()
{
	int ret;
	
	this->lock();
	ret = NumHarmony;
	this->unlock();
	
	return ret;
}

/******************************
******************************/
void THREAD_AMP_OF_FREQ::update_Amp(int Nth, double value)
{
	this->lock();
	
	Array_AmpOfFreq[Nth].set_Amp(value);
	
	this->unlock();
}

/******************************
******************************/
double THREAD_AMP_OF_FREQ::get_Amp(int Nth, int HarmonyId, bool b_HighDown)
{
	double ret;
	
	this->lock();
	ret = Array_AmpOfFreq[Nth].get_Amp(HarmonyId, b_HighDown);
	this->unlock();
	
	return ret;
}

/******************************
******************************/
double THREAD_AMP_OF_FREQ::get_Freq(int Nth, int HarmonyId)
{
	double ret;
	
	this->lock();
	ret = Array_AmpOfFreq[Nth].get_Freq(HarmonyId);
	this->unlock();
	
	return ret;
}

