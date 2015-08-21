#include "ofApp.h"

void ofApp::setup() {
    
    // hide mouse cursor
    ofHideCursor();
    
    ofBackground(0);
    ofSetWindowTitle("Image Player");
    
    vocal.loadSound("sound/audio.mp3");
    
    ofSetColor(0);
    
    /*
    // load filters
    dir.allowExt("cube");
    dir.listDir("LUTs/");
    dir.sort();
    if (dir.size()>0) {
        dirLoadIndex=0;
        // loadLUT(dir.getPath(dirLoadIndex));
        doLUT = true;
    }else{
        doLUT = false;
    }
    */
    
    dir.allowExt("jpg");
    int nFiles = dir.listDir("frames");
    if(nFiles) {
        
        images.resize(dir.numFiles());
        
        for(int i = 0; i < dir.numFiles(); i++) {
            
            // add the image to the vector
            string filePath = dir.getPath(i);
            
            // read the directory for the images
            // we know that they are named in seq
            loader.loadFromDisk(images[i], filePath);
            
            // images.push_back(ofImage());
            // images.back().loadImage(filePath);
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
    
    // set up video recording
    fileName = "";
    fileExt = ".mov"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats
    
    // override the default codecs if you like
    // run 'ffmpeg -codecs' to find out what your implementation supports (or -formats on some older versions)
    vidRecorder.setVideoCodec("mpeg4");
    vidRecorder.setVideoBitrate("4000k");
    vidRecorder.setAudioCodec("mp3");
    vidRecorder.setAudioBitrate("192k");
    
    soundStream.listDevices();
    soundStream.setDeviceID(2);
    sampleRate = 44100;
    channels = 2;
    soundStream.setup(this, 0, channels, sampleRate, 256, 4);

    vidRecorder.setup(fileName+ofGetTimestampString()+fileExt, 1920, 1080, appFPS, sampleRate, channels);
}

void ofApp::exit() {
    vidRecorder.close();
}

//--------------------------------------------------------------
void ofApp::update() {
    if(bRecording){ // && vidGrabber.isFrameNew()
        vidRecorder.addFrame( images[frameIndex] );
    }
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
                vocal.stop();
                vidRecorder.close();
                startPlayback = false;
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
            
            // start recording
            bRecording = true;
            vidRecorder.start();
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

void ofApp::audioIn(float *input, int bufferSize, int nChannels){
    if(bRecording)
        vidRecorder.addAudioSamples(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::loadLUT(string path){
    LUTloaded=false;
    
    ofFile file(path);
    string line;
    for(int i = 0; i < 5; i++) {
        getline(file, line);
        ofLog() << "Skipped line: " << line;
    }
    for(int z=0; z<32; z++){
        for(int y=0; y<32; y++){
            for(int x=0; x<32; x++){
                ofVec3f cur;
                file >> cur.x >> cur.y >> cur.z;
                lut[x][y][z] = cur;
            }
        }
    }
    
    LUTloaded = true;
}

//--------------------------------------------------------------
void ofApp::applyLUT(ofPixelsRef pix){
    if (LUTloaded) {
        
        for(int y = 0; y < pix.getHeight(); y++){
            for(int x = 0; x < pix.getWidth(); x++){
                
                ofColor color = pix.getColor(x, y);
                
                int lutPos [3];
                for (int m=0; m<3; m++) {
                    lutPos[m] = color[m] / 8;
                    if (lutPos[m]==31) {
                        lutPos[m]=30;
                    }
                }
                
                ofVec3f start = lut[lutPos[0]][lutPos[1]][lutPos[2]];
                ofVec3f end = lut[lutPos[0]+1][lutPos[1]+1][lutPos[2]+1];
                
                for (int k=0; k<3; k++) {
                    float amount = (color[k] % 8) / 8.0f;
                    color[k]= (start[k] + amount * (end[k] - start[k])) * 255;
                }
                
                images[frameIndex].setColor(x, y, color);
                
            }			
        }
        
        images[frameIndex].update();
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
    if (key == 'r') {
        bRecording = true;
    }
    if (key == 'c'){
        bRecording = false;
        vidRecorder.close();
    }
    if (key == 'j'){
        frameNum = 10000;
    }
    if (key == OF_KEY_UP){
        dirLoadIndex++;
        if (dirLoadIndex>=(int)dir.size()) {
            dirLoadIndex=0;
        }
        loadLUT(dir.getPath(dirLoadIndex));
    }
    if (key == OF_KEY_DOWN) {
        dirLoadIndex--;
        if (dirLoadIndex<0) {
            dirLoadIndex=dir.size()-1;
        }
        loadLUT(dir.getPath(dirLoadIndex));
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
