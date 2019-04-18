/************************************************************
************************************************************/
#include "th_fft.h"
#include "stdlib.h"

/************************************************************
************************************************************/

/******************************
******************************/
THREAD_FFT::THREAD_FFT()
: N(AUDIO_BUF_SIZE)
, LastInt(0)
{
	/********************
	********************/
	/* 窓関数 */
	fft_window.resize(N);
	for(int i = 0; i < N; i++)	fft_window[i] = 0.5 - 0.5 * cos(2 * PI * i / N);
	
	sintbl.resize(N + N/4);
	bitrev.resize(N);
	
	make_bitrev();
	make_sintbl();
	
	/********************
	********************/
	setup();
}

/******************************
******************************/
THREAD_FFT::~THREAD_FFT()
{
}

/******************************
******************************/
void THREAD_FFT::threadedFunction()
{
	while(isThreadRunning()) {
		lock();
		
		unlock();
		
		
		sleep(10);
	}
}

/******************************
******************************/
void THREAD_FFT::exit()
{
	this->lock();
	
	this->unlock();
}

/******************************
******************************/
void THREAD_FFT::setup()
{
	this->lock();
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		Gain[i] = 0;
	}
	this->unlock();
}

/******************************
******************************/
void THREAD_FFT::update()
{
	this->lock();
	
	this->unlock();
}


/******************************
******************************/
void THREAD_FFT::Log()
{
	/*
	for(int i = 0; i < AUDIO_BUF_SIZE/2; i++){
		fprintf(fp_Log, "%d,%f\n", i, ZeroCross__SlowSmoothedGain[i]);
	}
	*/
}

/******************************
description
	昇順
******************************/
int THREAD_FFT::double_sort( const void * a , const void * b )
{
	if(*(double*)a < *(double*)b){
		return -1;
	}else if(*(double*)a == *(double*)b){
		return 0;
	}else{
		return 1;
	}
}

/******************************
******************************/
double THREAD_FFT::getArrayVal(int id)
{
	if(AUDIO_BUF_SIZE/2 <= id) return 0;
	
	double ret = 0;
	
	this->lock();
	ret = Gain[id];
	this->unlock();
	
	return ret;
}

/******************************
******************************/
double THREAD_FFT::getArrayVal_x_DispGain(int id, float Gui_DispGain)
{
	if(AUDIO_BUF_SIZE/2 <= id) return 0;
	
	double ret = 0;
	
	this->lock();
	ret = ofMap(Gain[id], 0, Gui_DispGain, 0, FBO_HEIGHT, true);
	this->unlock();
	
	return ret;
}

/******************************
******************************/
void THREAD_FFT::update__Gain(const vector<float> &AudioSample)
{
	const float duration = 15.0;
	
	this->lock();
		/********************
		********************/
		float now = ofGetElapsedTimef();
		// if(fp_Log_fft != NULL) { if(now < duration) fprintf(fp_Log_fft, "%f,", now); }
			
		/********************
		********************/
		AudioSample_fft_LPF_saveToArray(AudioSample, now - LastInt);
		LastInt = now;
			
		/********************
		********************/
		/*
		if(fp_Log_fft != NULL){
			if(now < duration)	{ fprintf(fp_Log_fft, "%f\n", ofGetElapsedTimef()); }
			else				{ fclose(fp_Log_fft); fp_Log_fft = NULL; }
		}
		*/
	this->unlock();
}

/******************************
******************************/
void THREAD_FFT::AudioSample_fft_LPF_saveToArray(const vector<float> &AudioSample, float dt)
{
	/********************
	********************/
	if( AudioSample.size() != N ) { ERROR_MSG(); std::exit(1); }
	
	/********************
	********************/
	double x[N], y[N];
	
	for(int i = 0; i < N; i++){
		x[i] = AudioSample[i] * fft_window[i];
		y[i] = 0;
	}
	
	fft(x, y);

	/********************
	********************/
	Gain[0] = 0;
	for(int i = 1; i < N/2; i++){
		double GainTemp = 2 * sqrt(x[i] * x[i] + y[i] * y[i]);
		
		Gain[i] = LPF(Gain[i], GainTemp, Gui_Global->LPFAlpha_dt__FFTGain, dt);
		Gain[N - i] = Gain[i]; // 共役(yの正負反転)だが、Gainは同じ
	}
}

/******************************
******************************/
int THREAD_FFT::fft(double x[], double y[], int IsReverse)
{
	/*****************
		bit反転
	*****************/
	int i, j;
	for(i = 0; i < N; i++){
		j = bitrev[i];
		if(i < j){
			double t;
			t = x[i]; x[i] = x[j]; x[j] = t;
			t = y[i]; y[i] = y[j]; y[j] = t;
		}
	}

	/*****************
		変換
	*****************/
	int n4 = N / 4;
	int k, ik, h, d, k2;
	double s, c, dx, dy;
	for(k = 1; k < N; k = k2){
		h = 0;
		k2 = k + k;
		d = N / k2;

		for(j = 0; j < k; j++){
			c = sintbl[h + n4];
			if(IsReverse)	s = -sintbl[h];
			else			s = sintbl[h];

			for(i = j; i < N; i += k2){
				ik = i + k;
				dx = s * y[ik] + c * x[ik];
				dy = c * y[ik] - s * x[ik];

				x[ik] = x[i] - dx;
				x[i] += dx;

				y[ik] = y[i] - dy;
				y[i] += dy;
			}
			h += d;
		}
	}

	/*****************
	*****************/
	if(!IsReverse){
		for(i = 0; i < N; i++){
			x[i] /= N;
			y[i] /= N;
		}
	}

	return 0;
}

/******************************
******************************/
void THREAD_FFT::make_bitrev(void)
{
	int i, j, k, n2;

	n2 = N / 2;
	i = j = 0;

	for(;;){
		bitrev[i] = j;
		if(++i >= N)	break;
		k = n2;
		while(k <= j)	{j -= k; k /= 2;}
		j += k;
	}
}

/******************************
******************************/
void THREAD_FFT::make_sintbl(void)
{
	for(int i = 0; i < N + N/4; i++){
		sintbl[i] = sin(2 * PI * i / N);
	}
}


