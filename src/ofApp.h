#pragma once

#include "ofMain.h"
#include "ofxThreadedImageLoader.h"
#include "ofxVideoRecorder.h"
#include "ofxJSON.h"
#include <time.h>
#include <iomanip>
#include <fstream>

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
    void imageLoading(int i, bool init = false);
    vector<ofImage> images;
    ofImage imageBuffer;
    string imageFolder;
    int numImages;
    int bufferSize;
    
    
    // for video recording
    void setupVideoRecording();
    ofxVideoRecorder    vidRecorder;
    ofSoundStream       soundStream;
    void audioIn(float * input, int bufferSize, int nChannels);
    bool bRecording = false;
    int sampleRate;
    int channels;
    string fileName;
    string fileExt;
    
    // image filters
    int dirLoadIndex;
    void applyFilter();
    
    // json
    ofxJSONElement json;
    void displaySubtitle(int frameNum, int x, int y);
    void getElapsedTime(stringstream& ss, float offsetF, float offsetM);
    void writeSrtFile(const stringstream& ss, bool init = false);
};
