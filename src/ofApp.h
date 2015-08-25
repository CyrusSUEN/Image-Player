#pragma once

#include "ofMain.h"
#include "ofxThreadedImageLoader.h"
#include "ofxVideoRecorder.h"

class ofApp : public ofBaseApp {
    
public:
    
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    int   appFPS;
    float sequenceFPS;
    bool  startPlayback;
    int frameIndex;
    int imagesIndex;
    int frameNum;
    bool showDebuggingInfo;
    int endingLastingFrameNum;
    int startingBufferFrameNum;
    
    stack<bool> meetsFPSrequirement;
    
    ofDirectory dirImg;
    ofSoundPlayer vocal;
    
    // use threadedImageloader for improved loading performance
    ofxThreadedImageLoader loader;
    void dynamicLoading(int i);
    ofImage imageBuffer;
    
    // for video recording
    void setupVideoRecording();
    ofxVideoRecorder    vidRecorder;
    ofSoundStream       soundStream;
    void audioIn(float * input, int bufferSize, int nChannels);
    bool bRecording;
    int sampleRate;
    int channels;
    string fileName;
    string fileExt;
    
    void loadFilers();
    ofDirectory dirLut;
    bool doLUT;
    int dirLoadIndex;
    bool LUTloaded;
    ofVec3f lut[32][32][32];
    void loadLUT(string path);
    void applyLUT(ofPixelsRef pix);
};
