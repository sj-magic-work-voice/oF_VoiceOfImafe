/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxSyphon.h"

#include "sj_common.h"
#include "th_AmpOfFreq.h"
#include "th_fft.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class Vec2_FROM_TO{
private:
	ofVec2f from;
	ofVec2f to;
	
	
public:
	void set(ofVec2f _from, ofVec2f _to){
		from = _from;
		to = _to;
	}
	
	ofVec2f get_current(double ratio){
		if(ratio < 0) ratio = 0;
		if(1 < ratio) ratio = 1;
		
		ofVec2f current;
		current.x = from.x + (to.x - from.x) * ratio;
		current.y = from.y + (to.y - from.y) * ratio;
		
		return current;
	}
};

/**************************************************
**************************************************/
struct VBO_SET : private Noncopyable{
	ofVbo Vbo;
	vector<ofVec3f> VboVerts;
	vector<ofFloatColor> VboColor;
	
	void setup(int size){
		VboVerts.resize(size);
		VboColor.resize(size);
		
		Vbo.setVertexData(&VboVerts[0], VboVerts.size(), GL_DYNAMIC_DRAW);
		Vbo.setColorData(&VboColor[0], VboColor.size(), GL_DYNAMIC_DRAW);
	}
	
	void set_singleColor(const ofColor& color){
		for(int i = 0; i < VboColor.size(); i++) { VboColor[i].set( double(color.r)/255, double(color.g)/255, double(color.b)/255, double(color.a)/255); }
	}
	
	void update(){
		Vbo.updateVertexData(&VboVerts[0], VboVerts.size());
		Vbo.updateColorData(&VboColor[0], VboColor.size());
	}
	
	void draw(int drawMode){
		Vbo.draw(drawMode, 0, VboVerts.size());
	}
	
	void draw(int drawMode, int total){
		if(VboVerts.size() < total) total = VboVerts.size();
		Vbo.draw(drawMode, 0, total);
	}
};

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	
	/****************************************
	****************************************/
	/********************
	********************/
	enum{
		NUM_FBOS = 4,
	};
	
	vector<ofImage> images;
	ofFbo fbo[NUM_FBOS];
	int id_image;
	
	ofFbo fbo_cal;
	ofPixels pix_image;
	
	THREAD_AMP_OF_FREQ* th_AmpOfFreq;
	
	/********************
	********************/
	enum STATE{
		STATE_PAUSE,
		STATE_ANALYZE,
		STATE_ROTATION,
	};
	
	STATE State;
	float t_From;
	double CursorPos;
	double sgn_Cursor;
	
	Vec2_FROM_TO FboPos[NUM_FBOS];
	float t_LastInt;
	
	bool b_DispGui;
	bool b_NextImage;
	
	bool b_DispFrameRate;
	
	/********************
	********************/
	ofSoundStream soundStream;
	int soundStream_Input_DeviceId;
	int soundStream_Output_DeviceId;
	
	vector<float> AudioSample;
	
	ofTrueTypeFont font;
	
	THREAD_FFT* fft_thread;
	VBO_SET Vboset_FreqBased;
	
	float t_ResetAudio_from;
	
	/********************
	********************/
	ofxSyphonServer mainOutputSyphonServer;
	ofxSyphonServer SyphonServer_0;
	
	/****************************************
	****************************************/
	void makeup_image_list(const string dirname, vector<ofImage>& images);
	void Refresh_fboContents();
	int get_NthNextId_of_images(int Nth);
	void setup_Gui();
	void StateChart(float now);
	void RefreshVerts();
	void draw_FreqBasedGraph(const ofColor& col_Back, const ofRectangle& rect_Back, const ofPoint& Coord_zero, float Screen_y_max, float Val_Disp_y_Max);
	bool Is_CursorEnd();
	bool ResetCursorPos();
	void Reset_SoundStream();
	void publish_syphon();
	
public:
	/****************************************
	****************************************/
	ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId);
	~ofApp();

	void setup();
	void update();
	void draw();
	void exit();
	
	void audioIn(ofSoundBuffer & buffer);
	void audioOut(ofSoundBuffer & buffer);
	
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
