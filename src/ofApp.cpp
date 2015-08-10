#include "ofApp.h"

void ofApp::setup() {
    
    // hide mouse cursor
    ofHideCursor();
    
    ofBackground(0);
    ofSetWindowTitle("Image Player");
    
    vocal.loadSound("sound/audio.mp3");
    
    ofSetColor(0);
    
    int nFiles = dir.listDir("frames");
    if(nFiles) {
        
        images.resize(dir.numFiles());
        
        for(int i = 0; i < dir.numFiles(); i++) {
            
            // add the image to the vector
            string filePath = dir.getPath(i);
            
            // read the directory for the images
            // we know that they are named in seq
            loader.loadFromDisk(images[i], filePath);
        }
    }
    else printf("Could not find folder\n");
    
    showDebuggingInfo = false;
    startPlayback = true;
    
    sequenceFPS = 30;
    
    // set the app fps
    appFPS = 30;
    ofSetFrameRate(appFPS);
    
    // number of frames spent when reached the ending image
    endingLastingFrameNum = 0;
    
    // number of frames spent before playback
    startingBufferFrameNum = 0;
    
    // number of frame required that passes appFPS (default 30fps) before playback
    int numFrameRateCheck = 5;
    for (int i = 0; i < numFrameRateCheck; i++) {
        meetsFPSrequirement.push(false);
    }
    
    // internal variables; Do not modify
    frameIndex = -1;
    imagesIndex = 0;
    frameNum = -1 - startingBufferFrameNum;
}

//--------------------------------------------------------------
void ofApp::update() {
    
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    // we need some images if not return
    if((int)images.size() <= 0) {
        ofSetColor(255);
        ofDrawBitmapString("No Images...", ofGetWidth()/2-30, ofGetHeight()/2);
        return;
    }

    if (!meetsFPSrequirement.empty()) {
        if (ofGetFrameRate() >= appFPS)
            meetsFPSrequirement.pop();
    }
    
    if (startPlayback && meetsFPSrequirement.empty()) {
        
        // every draw() call means ofGetFrameNum() is increased by 1
        frameNum++; // = ofGetFrameNum();
        
        if ( frameNum == ofToInt( dir.getName( imagesIndex ) ) ) {
            frameIndex = imagesIndex;
            imagesIndex++;
            if (imagesIndex >= images.size()) {
                if (endingLastingFrameNum)
                    frameNum = -endingLastingFrameNum - 1;
                else
                    frameNum = -1;
                imagesIndex = 0;
            }
        }
        
        if (frameIndex < 0) {
            return;
        }
        
        // at the start of the image sequence
        if (frameIndex == 0) {
            vocal.play();
            
            // workaround to no image will be drawn
            ofSetColor(255);
        }
        
        if (!showDebuggingInfo) {
            images[frameIndex].draw(0, 0);
        }
        else {
            // draw the image sequence at the new frame count
            images[frameIndex].draw(256, 36);
            
            // how fast is the app running and some other info
            ofSetColor(50);
            ofRect(0, 0, 200, 75);
            ofSetColor(200);
            string info;
            info += ofToString(frameIndex) + "/" + ofToString(images.size()-1) + " file index\n";
            info += ofToString(appFPS)+"/"+ofToString(ofGetFrameRate(), 0)+" current fps\n";
            info += ofToString(sequenceFPS)+" target sequence fps\n\n";
            info += ofToString(vocal.getPosition() * 100)+"% audio position";
            
            ofDrawBitmapString(info, 15, 20);
        }
    }
    else if (!startPlayback && meetsFPSrequirement.empty()) {
        ofSetColor(255);
        ofDrawBitmapString("Press spacebar to start playing", ofGetWidth()/2-100, ofGetHeight()/2);
        
        frameNum = -1;
        imagesIndex = 0;
        vocal.stop();
    }
    else {
        ofBackground(0);
        return;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        if (meetsFPSrequirement.empty())
            startPlayback = !startPlayback;
    }
    if (key == 'd') {
        showDebuggingInfo = !showDebuggingInfo;
    }
    if (key == 'f') {
        ofToggleFullscreen();
        ofSetWindowTitle("Image Player");
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
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
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
