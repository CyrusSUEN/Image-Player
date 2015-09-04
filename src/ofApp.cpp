#include "ofApp.h"

void ofApp::setup() {
    
    // hide mouse cursor
    ofHideCursor();
    
    ofBackground(0);
    ofSetWindowTitle("Image Player");
    
    // load sound
    vocal.load("sound/audio.mp3");
    
    dirImg.allowExt("jpg");
    imageFolder = "frames_";
    imageFolder += "abstract";
    
    // load images
    bufferSize = 5;
    numImages = dirImg.listDir(imageFolder);
    if (numImages > bufferSize)
        images.resize(bufferSize);
    else
        images.resize(numImages);
    
    for (int i = 0; i < images.size(); i++) {
        imageLoading(i, true);
    }
    
    // load JSON
    if (json.open("phraseCounts.json"))
        ; // ofLogNotice("ofApp::setup") << json.getRawString();
    
    showDebuggingInfo = true;
    startPlayback = true;
    
    sequenceFPS = 30;
    
    // set the app fps
    appFPS = 30;
    ofSetFrameRate(appFPS);
    
    // TODO seems not working right now
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
    setupVideoRecording();
}

void ofApp::exit() {
    vidRecorder.close();
}

void ofApp::setupVideoRecording() {
    fileName = imageFolder;
    fileExt = ".mov"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats
    
    // override the default codecs if you like
    // run 'ffmpeg -codecs' to find out what your implementation supports (or -formats on some older versions)
    vidRecorder.setVideoCodec("mpeg4");
    vidRecorder.setVideoBitrate("4000k");
    vidRecorder.setAudioCodec("mp3");
    vidRecorder.setAudioBitrate("192k");
    
    // set wavTap as the sound input source
    for (auto i : soundStream.getDeviceList()) {
        if (i.name == "WavTap: WavTap") {
            soundStream.setDeviceID(i.deviceID);
            break;
        }
    }
    
    sampleRate = 44100;
    channels = 2;
    soundStream.setup(this, 0, channels, sampleRate, 256, 4);
    
    vidRecorder.setup(fileName+"-"+ofGetTimestampString()+fileExt, 1920, 1080, appFPS, sampleRate, channels);
}

void ofApp::imageLoading(int i, bool init) {
    
    imageBuffer = images[(i) % bufferSize];
    
    int dirIndex = 0;
    if (!init)
        dirIndex = bufferSize;
    
    if (numImages > (i + dirIndex)) {
        loader.loadFromDisk(images[i % bufferSize], dirImg.getPath(i + dirIndex));
        // imageBuffer.loadImage(dirImg.getPath(i));
        
        /*
        images[i % bufferSize] = ofImage();
        images[i % bufferSize].load(dirImg.getPath(i));
        imageBuffer = images[(i) % bufferSize];
         */
        
        //applyFilter();
    }
    
}

void ofApp::displaySubtitle(int frameNum, int x, int y) {
    static int frameCount = 0;
    static int frameLasts = 0;
    static int playedImagesIndex = -1;
    static string subtitle = "";
    
    static stringstream ss;
    static int srtIndex = 0;
    static float offsetF = 0;
    static float offsetM = 0;
    
    static int index = 0;
    static bool isStarted = false;
    
    // start displaying subtitle
    if (!isStarted && frameNum == 0) {
        isStarted = true;
        
        // resetting static function variables
        frameCount = 0;
        frameLasts = 0;
        index = 0;
        srtIndex = 0;
        offsetF = ofGetElapsedTimef();
        offsetM = ofGetElapsedTimeMillis();
        
        // reset srt file content
        writeSrtFile(ss, true);
    }
    // case: increase index
    else if (isStarted && frameLasts != 0 && frameCount == frameLasts) {
        {
            ss << " --> ";
            getElapsedTime(ss, offsetF, offsetM);
            ss << endl << subtitle << endl << endl;
            
            writeSrtFile(ss);
            
            ss.str(""); // clear the content
        }
        
        frameCount = 0;
        frameLasts = 0;
        index++;
    }
    
    if (isStarted) {
         // frame 0 starting to display subtitle
        if (frameCount == 0 && frameLasts == 0) {
            // terminate the subtitle function if index out of bound
            if (index >= json.size()) {
                isStarted = false;
            }
            
            subtitle = json[index]["text"].asString();
            
            // check if this should be skipped
            while (isStarted && subtitle.find_first_of(" ") == string::npos) {
                index++;
                ofLogNotice("displaySubtitle skippable caption found") << index;
                
                // terminate the subtitle function if index out of bound
                if (index >= json.size()) {
                    isStarted = false;
                }
                else
                    subtitle = json[index]["text"].asString();
            }
            
            frameLasts = json[index]["frames"].asInt();
            
            {
                ss << ++srtIndex << endl;
                getElapsedTime(ss, offsetF, offsetM);
            }
        }
        // continue to display subtitle
        if (frameCount < frameLasts) { // draw subtitle
            frameCount++;
            ofDrawBitmapString(subtitle, x, y);
        }
    }
    
    if (showDebuggingInfo) {
        string info;
        
        info += ofToString(frameCount) + " frameCount\n";
        info += ofToString(frameLasts) + " frameLasts\n";
        
        ofDrawBitmapString(info, 15, 180);
    }
}

void ofApp::getElapsedTime(stringstream& ss, float offsetF, float offsetM) {
    time_t seconds(ofGetElapsedTimef() - offsetF); // convert ofGetElapsedTimef into time_t
    tm *p = gmtime(&seconds); // convert to broken down time
    
    unsigned long long timeMillis = ofGetElapsedTimeMillis() - offsetM;
    
    ss << setfill('0') << setw(2) << p->tm_hour << ":"
    << setw(2) << p->tm_min << ":" << setw(2) << p->tm_sec
    << "," << setw(3) << timeMillis%1000;
}

void ofApp::writeSrtFile(const stringstream& ss, bool init) {
    
    if (init) {
        ofstream fout;
        fout.open(ofToDataPath(imageFolder + ".srt").c_str(), ios::out | ios::trunc);
        fout.close();
    }
    else {
        ofstream fout;
        fout.open(ofToDataPath(imageFolder + ".srt").c_str(), ios::out | ios::in);
        fout.seekp(0, ios::end); // start writing at the end; requires ios::in for seeking EOL
        fout << ss.str();
        fout.close();
    }
}

void ofApp::applyFilter() {
    // set the tint color
    ofColor filterColor = ofColor::green;
    
    for(auto line: imageBuffer.getPixels().getLines()){
        for(auto pixel: line.getPixels()){
            float tintPercentage = .25;
            pixel[0] = pixel[0] + (tintPercentage * (filterColor.r - pixel[0]));
            pixel[1] = pixel[1] + (tintPercentage * (filterColor.g - pixel[1]));
            pixel[2] = pixel[2] + (tintPercentage * (filterColor.b - pixel[2]));
        }
    }
    imageBuffer.update();
}

//--------------------------------------------------------------
void ofApp::update() {
    if(bRecording){ // && vidGrabber.isFrameNew()
        vidRecorder.addFrame( imageBuffer );
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    // we need some images if not return
    if((int)dirImg.size() <= 0) {
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
        
        if ( frameNum == ofToInt( dirImg.getName( imagesIndex ) ) ) {
            frameIndex = imagesIndex;
            imageLoading(imagesIndex);
            applyFilter();
            imagesIndex++;
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
            imageBuffer.draw(0, 0);
            displaySubtitle(frameNum, 15, 20);
        }
        else {
            // draw the image sequence at the new frame count
            imageBuffer.draw(256, 36);
            displaySubtitle(frameNum, 15, 200);
            
            // how fast is the app running and some other info
            ofSetColor(50);
            ofRect(0, 0, 200, 75);
            ofSetColor(200);
            string info;
            info += ofToString(frameIndex) + "/" + ofToString(dirImg.size()-1) + " file index\n";
            info += ofToString(appFPS)+"/"+ofToString(ofGetFrameRate(), 0)+" current fps\n";
            info += ofToString(sequenceFPS)+" target sequence fps\n\n";
            info += ofToString(vocal.getPosition() * 100)+"% audio position\n";
            info += ofToString(frameNum) + " frame number\n";
            
            ofDrawBitmapString(info, 15, 20);
        }
        
        // update environment variables
        if (imagesIndex >= dirImg.size()) {
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
