#pragma once

#include "ofMain.h"
#include "ofxThreadedImageLoader.h"

class ofApp : public ofBaseApp {
    
public:
    
    void setup();
    void update();
    void draw();
    
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
    
    ofDirectory dir;
    ofSoundPlayer vocal;
    vector <ofImage> images;
    
    // use threadedImageloader for improved loading performance
    ofxThreadedImageLoader loader;
};
