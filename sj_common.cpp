/************************************************************
************************************************************/
#include "sj_common.h"

/************************************************************
************************************************************/
/********************
********************/
int GPIO_0 = 0;
int GPIO_1 = 0;

/********************
********************/
GUI_GLOBAL* Gui_Global = NULL;

FILE* fp_Log = NULL;
FILE* fp_Log_main = NULL;
FILE* fp_Log_Audio = NULL;
FILE* fp_Log_fft = NULL;


/************************************************************
func
************************************************************/
/******************************
******************************/
double LPF(double LastVal, double CurrentVal, double Alpha_dt, double dt)
{
	double Alpha;
	if((Alpha_dt <= 0) || (Alpha_dt < dt))	Alpha = 1;
	else									Alpha = 1/Alpha_dt * dt;
	
	return CurrentVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
double LPF(double LastVal, double CurrentVal, double Alpha)
{
	if(Alpha < 0)		Alpha = 0;
	else if(1 < Alpha)	Alpha = 1;
	
	return CurrentVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
double sj_max(double a, double b)
{
	if(a < b)	return b;
	else		return a;
}


/************************************************************
class
************************************************************/

/******************************
******************************/
void GUI_GLOBAL::setup(string GuiName, string FileName, float x, float y)
{
	/********************
	********************/
	gui.setup(GuiName.c_str(), FileName.c_str(), x, y);
	
	/********************
	********************/
	Group_Sound.setup("Sound");
		Group_Sound.add(vol.setup("vol", 0.05, 0, 3.0));
		Group_Sound.add(NumHarmony.setup("NumHarmony", 2, 1, 3));
		Group_Sound.add(CursorSpeed.setup("CursorSpeed", 80.0, 70.0, 100.0));
		Group_Sound.add(d_ROTATION.setup("d_ROTATION", 0.6, 0.0, 2.0));
		Group_Sound.add(d_PAUSE.setup("d_PAUSE", 0.1, 0.0, 5.0));
		Group_Sound.add(b_HighDown.setup("HighFreq Cut", true));
		Group_Sound.add(b_ImageChange.setup("b_ImageChange", true));
		Group_Sound.add(b_PALINDROME.setup("b_PALINDROME", true));
	gui.add(&Group_Sound);
	
	Group_FFT.setup("fft");
		Group_FFT.add(LPFAlpha_dt__FFTGain.setup(">LPF_dt", 0.15, 0, 2.0));
		Group_FFT.add(Val_DispMax__FFTGain.setup(">ValDisp FFT", 0.05, 0, 0.1));
	gui.add(&Group_FFT);

	/********************
	********************/
	gui.minimizeAll();
}

