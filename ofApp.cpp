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

/* */

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId)
: id_image(0)
, th_AmpOfFreq(THREAD_AMP_OF_FREQ::getInstance())
, fft_thread(THREAD_FFT::getInstance())
, State(STATE_PAUSE)
, t_From(0)
, CursorPos(0)
, sgn_Cursor(1)
, t_LastInt(0)
, soundStream_Input_DeviceId(_soundStream_Input_DeviceId)
, soundStream_Output_DeviceId(_soundStream_Output_DeviceId)
, b_DispGui(true)
, b_NextImage(false)
, b_DispFrameRate(false)
, t_ResetAudio_from(-1)
, b_ExchangeByHsv(false)
{
	fp_Log			= fopen("../../../data/Log.csv", "w");
	fp_Log_main		= fopen("../../../data/Log_main.csv", "w");
	fp_Log_Audio 	= fopen("../../../data/Log_Audio.csv", "w");
	fp_Log_fft 		= fopen("../../../data/Log_fft.csv", "w");
	
	FboPos[0].set(ofVec2f(0, 0), ofVec2f(-FBO_WIDTH, 0));
	FboPos[1].set(ofVec2f(0, FBO_HEIGHT), ofVec2f(0, 0));
	FboPos[2].set(ofVec2f(FBO_WIDTH, FBO_HEIGHT), ofVec2f(0, FBO_HEIGHT));
	FboPos[3].set(ofVec2f(FBO_WIDTH, 0), ofVec2f(FBO_WIDTH, FBO_HEIGHT));
	
	font.load("RictyDiminished-Regular.ttf", 8, true, true, true);
	
	Vboset_FreqBased.setup(AUDIO_BUF_SIZE/2 * 4); /* square */
	Vboset_FreqBased.set_singleColor(ofColor(255, 255, 255, 100));
}

/******************************
******************************/
ofApp::~ofApp()
{
	if(fp_Log)			fclose(fp_Log);
	if(fp_Log_main)		fclose(fp_Log_main);
	if(fp_Log_Audio)	fclose(fp_Log_Audio);
	if(fp_Log_fft)		fclose(fp_Log_fft);
}

/******************************
******************************/
void ofApp::exit(){
	/********************
	ofAppとaudioが別threadなので、ここで止めておくのが安全.
	********************/
	soundStream.stop();
	soundStream.close();
	
	/********************
	********************/
	th_AmpOfFreq->exit();
	try{
		/********************
		stop済みのthreadをさらにstopすると、Errorが出るようだ。
		********************/
		while(th_AmpOfFreq->isThreadRunning()){
			th_AmpOfFreq->waitForThread(true);
		}
		
	}catch(...){
		printf("Thread exiting Error\n");
	}

	/********************
	********************/
	fft_thread->exit();
	try{
		/********************
		stop済みのthreadをさらにstopすると、Errorが出るようだ。
		********************/
		while(fft_thread->isThreadRunning()){
			fft_thread->waitForThread(true);
		}
		
	}catch(...){
		printf("Thread exiting Error\n");
	}
	
	/********************
	********************/
	printf("\n> Good bye\n");
}	

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("Voice of Image");
	
	ofSetWindowShape( WINDOW_WIDTH, WINDOW_HEIGHT );
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	mainOutputSyphonServer.setName("Voice_of_image"); // server name
	SyphonServer_0.setName("Voice_of_image_0");
	
	setup_Gui();
	
	/********************
	********************/
	makeup_image_list("../../../data/image", images);
	
	/********************
	********************/
	for(int i = 0; i < NUM_FBOS; i++) { fbo[i].allocate(FBO_WIDTH, FBO_HEIGHT, GL_RGBA); }
	fbo_cal.allocate(FBO_CAL_WIDTH, FBO_CAL_HEIGHT, GL_RGBA);
	pix_image.allocate(FBO_CAL_WIDTH, FBO_CAL_HEIGHT, GL_RGBA);
	
	Refresh_fboContents();
	
	/********************
	********************/
	th_AmpOfFreq->setup();
	
	/********************
	********************/
	AudioSample.assign(AUDIO_BUF_SIZE, 0.0);
	fft_thread->setup();
	RefreshVerts();
	
	/********************
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	********************/
	soundStream.printDeviceList();
	
	/********************
	soundStream.setup()の位置に注意:最後
		setup直後、audioIn()/audioOut()がstartする.
		これらのmethodは、fft_threadにaccessするので、start前にReStart()によって、fft_threadが初期化されていないと、不正accessが発生してしまう.
	********************/
	Reset_SoundStream();
}

/******************************
******************************/
void ofApp::Reset_SoundStream()
{
	/********************
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	********************/
	ofSoundStreamSettings settings;
	
	if( soundStream_Output_DeviceId == -1 ){
		ofExit();
		return;
		
	}else{
		vector<ofSoundDevice> devices = soundStream.getDeviceList();
		
		if( soundStream_Input_DeviceId != -1 ){
			settings.setInDevice(devices[soundStream_Input_DeviceId]);
			settings.setInListener(this);
			settings.numInputChannels = AUDIO_BUFFERS;
		}else{
			settings.numInputChannels = 0;
		}
		
		if( soundStream_Output_DeviceId != -1 ){
			settings.setOutDevice(devices[soundStream_Output_DeviceId]);
			settings.numOutputChannels = AUDIO_BUFFERS;
			settings.setOutListener(this); /* Don't forget this */
		}else{
			settings.numOutputChannels = 0;
		}
		
		settings.numBuffers = 4;
		settings.sampleRate = AUDIO_SAMPLERATE;
		settings.bufferSize = AUDIO_BUF_SIZE;
	}
	
	/********************
	soundStream.setup()の位置に注意:最後
		setup直後、audioIn()/audioOut()がstartする.
		これらのmethodは、fft_threadにaccessするので、start前にReStart()によって、fft_threadが初期化されていないと、不正accessが発生してしまう.
	********************/
	soundStream.setup(settings);
	// soundStream.start();
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
	Gui_Global->setup("Void", "gui.xml", 1000, 10);
}

/******************************
******************************/
void ofApp::RefreshVerts()
{
	/********************
	********************/
	float BarWidth = GRAPH_BAR_WIDTH__FFT_GAIN;
	float BarSpace = GRAPH_BAR_SPACE__FFT_GAIN;
	
	/********************
	********************/
	float Gui_DispGain = Gui_Global->Val_DispMax__FFTGain;
	for(int j = 0; j < AUDIO_BUF_SIZE/2; j++){
		Vboset_FreqBased.VboVerts[j * 4 + 0].set( BarSpace * j            , 0 );
		Vboset_FreqBased.VboVerts[j * 4 + 1].set( BarSpace * j            , fft_thread->getArrayVal_x_DispGain(j, Gui_DispGain) );
		Vboset_FreqBased.VboVerts[j * 4 + 2].set( BarSpace * j  + BarWidth, fft_thread->getArrayVal_x_DispGain(j, Gui_DispGain) );
		Vboset_FreqBased.VboVerts[j * 4 + 3].set( BarSpace * j  + BarWidth, 0 );
	}
}

/******************************
******************************/
void ofApp::Refresh_fboContents()
{
	/********************
	********************/
	id_image = get_NthNextId_of_images(1);
	
	for(int i = 0; i < NUM_FBOS; i++){
		fbo[i].begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		images[get_NthNextId_of_images(i)].draw(0, 0, fbo[i].getWidth(), fbo[i].getHeight());
		fbo[i].end();
	}
	
	/********************
	********************/
	fbo_cal.begin();
	ofClear(0, 0, 0, 0);
	ofSetColor(255, 255, 255, 255);
	fbo[0].draw(0, 0, fbo_cal.getWidth(), fbo_cal.getHeight());
	fbo_cal.end();
	
	fbo_cal.readToPixels(pix_image);
}

/******************************
******************************/
int ofApp::get_NthNextId_of_images(int Nth)
{
	if(Nth <= 0) return id_image;
	
	int Next_id = id_image + Nth;
	Next_id = Next_id % images.size();
	
	return Next_id;
}

/******************************
******************************/
void ofApp::makeup_image_list(const string dirname, vector<ofImage>& images)
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
				if( (str[str.size()-1] == "png") || (str[str.size()-1] == "jpg") || (str[str.size()-1] == "jpeg") ){
					ofImage _image;
					_image.load(wPathName);
					images.push_back(_image);
				}
			}
		}
		
		pEnt = readdir( pDir ); // 次のファイルを検索する
	}

	closedir( pDir );
	
	/********************
	********************/
	if(images.size() < 3)	{ ERROR_MSG();std::exit(1);}
	else					{ printf("> %d images loaded\n", int(images.size())); fflush(stdout); }
}

/******************************
******************************/
void ofApp::update(){
	float now = ofGetElapsedTimef();
	
	fft_thread->update();
	th_AmpOfFreq->update_NumHarmony(int(Gui_Global->NumHarmony));
	
	StateChart(now);
	
	
	t_LastInt = now;
}

/******************************
******************************/
void ofApp::StateChart(float now){
	switch(State){
		case STATE_PAUSE:
			if(Gui_Global->d_PAUSE <= now - t_From){
				State = STATE_ANALYZE;
				t_From = now;
				ResetCursorPos();
			}else{
				for(int i = 0; i < FBO_CAL_HEIGHT; i++){
					th_AmpOfFreq->update_Amp(i, 0);
				}
			}
			break;
			
		case STATE_ANALYZE:
			CursorPos += sgn_Cursor * Gui_Global->CursorSpeed * (now - t_LastInt);
			
			if(b_NextImage){
				State = STATE_ROTATION;
				t_From = now;
				// th_AmpOfFreq->clear_Amp();
				
				b_NextImage = false;
			}else if( Is_CursorEnd() ){
				if(Gui_Global->b_ImageChange){
					State = STATE_ROTATION;
					t_From = now;
					// th_AmpOfFreq->clear_Amp();
				}else{
					if(Gui_Global->b_PALINDROME){
						sgn_Cursor = -sgn_Cursor;
					}else{
						if(0 < sgn_Cursor)	CursorPos = 0;
						else				CursorPos = FBO_WIDTH;
					}
				}
				
			}else{
				for(int i = 0; i < FBO_CAL_HEIGHT; i++){
					ofColor col = pix_image.getColor( CursorPos * (double(FBO_CAL_HEIGHT) / double(FBO_HEIGHT)), i );
					
					if(b_ExchangeByHsv){
						float hue = 0;
						float saturation = 0;
						float brightness = 0;
						col.getHsb(hue, saturation, brightness);
						
						th_AmpOfFreq->update_Amp(i, double(brightness)/255);
					}else{
						th_AmpOfFreq->update_Amp(i, 0.299 * (double)col.r/255 + 0.587 * (double)col.g/255 + 0.114 * (double)col.b/255);
					}
				}
			}
			break;
			
		case STATE_ROTATION:
			if(Gui_Global->d_ROTATION <= now - t_From){
				State = STATE_PAUSE;
				t_From = now;
				Refresh_fboContents();
			}else{
				for(int i = 0; i < FBO_CAL_HEIGHT; i++){
					th_AmpOfFreq->update_Amp(i, 0);
				}
			}
			break;
	}
}

/******************************
******************************/
bool ofApp::ResetCursorPos()
{
	if(0 < sgn_Cursor)	CursorPos = 0;
	else				CursorPos = FBO_WIDTH;
}

/******************************
******************************/
bool ofApp::Is_CursorEnd()
{
	if(0 < sgn_Cursor){
		if(FBO_WIDTH <= CursorPos)	return true;
		else						return false;
	}else{
		if(CursorPos <= 0)			return true;
		else						return false;
	}
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	float now = ofGetElapsedTimef();
	
	/********************
	********************/
	ofClear(0, 0, 0, 0);
	ofSetColor(255, 255, 255, 255);
	
	for(int i = NUM_FBOS - 1; 0 <= i; i--){
		ofVec2f pos;
		if(State == STATE_ROTATION) pos = FboPos[i].get_current( (now - t_From) / Gui_Global->d_ROTATION );
		else						pos = FboPos[i].get_current(0);
		
		if(i == 0)	ofSetColor(255, 255, 255, 255);
		else		ofSetColor(40);
		fbo[i].draw(pos.x, pos.y, fbo[i].getWidth(), fbo[i].getHeight());
	}
	
	if(State == STATE_ANALYZE){
		ofSetColor(255, 255, 0, 100);
		ofSetLineWidth(2.0);
		ofDrawLine(CursorPos, 0, CursorPos, FBO_HEIGHT);
	}
	
	/********************
	********************/
	RefreshVerts();
	
	// 以下は、audioOutからの呼び出しだと segmentation fault となってしまった.
	Vboset_FreqBased.update();
	
	
	/********************
	********************/
	ofRectangle rect_Back;
	ofPoint Coord_zero;
	float y_max;
	
	rect_Back.x = FBO_WIDTH;	rect_Back.y = 0;	rect_Back.width = FBO_WIDTH;	rect_Back.height = FBO_HEIGHT;
	Coord_zero.x = FBO_WIDTH;	Coord_zero.y = FBO_HEIGHT;
	y_max = FBO_HEIGHT;
	draw_FreqBasedGraph(ofColor(0, 0, 0, 255), rect_Back, Coord_zero, y_max, Gui_Global->Val_DispMax__FFTGain);
	
	/********************
	********************/
	publish_syphon();
	
	/********************
	********************/
	if(b_DispGui) Gui_Global->gui.draw();
	
	if(b_DispFrameRate){
		ofSetColor(255, 0, 0, 255);
		
		char buf[BUF_SIZE_S];
		sprintf(buf, "%5.1f", ofGetFrameRate());
		
		font.drawString(buf, 30, 30);
	}

}

/******************************
******************************/
void ofApp::publish_syphon()
{
	mainOutputSyphonServer.publishScreen();
	
	ofTexture tex = fbo[0].getTextureReference();
	SyphonServer_0.publishTexture(&tex);
}

/******************************
******************************/
void ofApp::draw_FreqBasedGraph(const ofColor& col_Back, const ofRectangle& rect_Back, const ofPoint& Coord_zero, float Screen_y_max, float Val_Disp_y_Max){
	/********************
	********************/
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	/********************
	back color.
	********************/
	ofSetColor(col_Back);
	ofDrawRectangle(rect_Back);
		
	/********************
	********************/
	ofPushStyle();
	ofPushMatrix();
		/********************
		********************/
		ofTranslate(Coord_zero);
		ofScale(1, -1, 1);
		
		/********************
		y目盛り
		********************/
		if(0 < Val_Disp_y_Max){
			const int num_lines = 5;
			const double y_step = Screen_y_max/num_lines;
			for(int i = 0; i < num_lines; i++){
				int y = int(i * y_step + 0.5);
				
				ofSetColor(ofColor(100));
				ofSetLineWidth(1);
				ofDrawLine(0, y, FBO_WIDTH - 1, y);
	
				/********************
				********************/
				char buf[BUF_SIZE_S];
				sprintf(buf, "%7.7f", Val_Disp_y_Max/num_lines * i);
				
				ofSetColor(ofColor(100));
				ofScale(1, -1, 1); // 文字が上下逆さまになってしまうので.
				font.drawString(buf, FBO_WIDTH - 1 - font.stringWidth(buf) - 10, -y); // y posはマイナス
				ofScale(1, -1, 1); // 戻す.
			}
		}
		
		/********************
		********************/
		ofSetColor(255);
		glPointSize(1.0);
		glLineWidth(1);
		
		Vboset_FreqBased.draw(GL_QUADS);
		
	ofPopMatrix();
	ofPopStyle();
}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case 'd':
			b_DispGui = !b_DispGui;
			break;
			
		case 'n':
			b_NextImage = true;
			break;
			
		case 'f':
			b_DispFrameRate = !b_DispFrameRate;
			break;
			
		case 'e':
			b_ExchangeByHsv = !b_ExchangeByHsv;
			
			if(b_ExchangeByHsv)	printf("HSV\n");
			else				printf("GRAY\n");
			fflush(stdout);
			
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

/******************************
audioIn/ audioOut
	同じthreadで動いている様子。
	また、audioInとaudioOutは、同時に呼ばれることはない(多分)。
	つまり、ofAppからaccessがない限り、変数にaccessする際にlock/unlock する必要はない。
	ofApp側からaccessする時は、threadを立てて、安全にpassする仕組みが必要(cf:NotUsed__thread_safeAccess.h)
	...ReferenceとSetで違う変数を用意し、このpassはthreadの向こう側で行う。
******************************/
void ofApp::audioIn(ofSoundBuffer & buffer){
	/*
	for (size_t i = 0; i < buffer.getNumFrames(); i++){
		vol_l[i] = buffer[i*2];
		vol_r[i] = buffer[i*2+1];
	}
	*/
}

/******************************
long		: 	8Byte
LONG_MAX	: 	9223372036854775807
				printf("%lld\n", LONG_MAX);
******************************/
void ofApp::audioOut(ofSoundBuffer & buffer){

	// #define FLOAT
	
#ifdef FLOAT
	/********************
	Not use this.
		512 + dt = 512
	になってしまうので.
	********************/
	static float t = 0.0;
	float dt = float(1.0)/ AUDIO_SAMPLERATE;
#else
	const long Boost = 1.0e+9;
	
	/********************
	毎回、ofGetElapsedTimef()で取得するよりも、こちらの方が綺麗に聞こえた.
	おそらく、dt に誤差があるから.
	********************/
	// static long t = 512 * Boost;
	static long t = 0 * Boost; 
	
	
	// long t = long(ofGetElapsedTimef() * Boost);
	
	long dt = long(1.0 * Boost/ AUDIO_SAMPLERATE);
#endif
	
	// float t_Enter = ofGetElapsedTimef();
	
	/********************
	for(int i = 0; i < buffer.size(); i++) { buffer[i] = 0; }
	********************/
	buffer.set(0);
	
	int NumHarmony = th_AmpOfFreq->get_NumHarmony();
	
	for (int i = 0; i < (int)buffer.getNumFrames(); i++){
		for(int j = 0; j < FBO_CAL_HEIGHT; j++){
			for(int k = 0; k < NumHarmony; k++){ // Harmony
				double Amp = th_AmpOfFreq->get_Amp(j, k, Gui_Global->b_HighDown);
				double f = th_AmpOfFreq->get_Freq(j, k);
				
				if(0 < Amp){
#ifdef FLOAT
					double _t = double(t + dt * i);
#else
					double _t = double(t + dt * i) / Boost;
#endif
					
					buffer[i * 2 + 0] += Gui_Global->vol * Amp * cos(TWO_PI * f * _t);
				}
			}
		}
	}
	
	t += dt * buffer.getNumFrames(); // for next Loop.
	
	for (int i = 0; i < (int)buffer.getNumFrames(); i++){
		buffer[i * 2 + 1] = buffer[i * 2 + 0];
		AudioSample[i] = buffer[i * 2 + 0];
	}
	
	/********************
	FFT Filtering
	1 process / block.
	********************/
	fft_thread->update__Gain(AudioSample);
	
	// fprintf(fp_Log, "%f\n", (ofGetElapsedTimef() - t_Enter) * 1000);
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
