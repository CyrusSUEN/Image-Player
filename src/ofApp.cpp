#include "ofApp.h"

void ofApp::setup() {
    
    // hide mouse cursor
    ofHideCursor();
    
    ofBackground(0);
    ofSetWindowTitle("Image Player");
    
    // load sound
    vocal.loadSound("sound/audio.mp3");
    
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
    
    showDebuggingInfo = false;
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
    
    soundStream.listDevices();
    soundStream.setDeviceID(2);
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
    }
    
}

void ofApp::loadFilers() {
    // load filters
    dirLut.allowExt("cube");
    dirLut.listDir("LUTs/");
    dirLut.sort();
    if (dirLut.size() > 0) {
        dirLoadIndex = 0;
        loadLUT(dirLut.getPath(dirLoadIndex));
        doLUT = true;
    }else{
        doLUT = false;
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
        
        cout << "Rewrite srt file" << endl;
        ofstream fout;
        fout.open(ofToDataPath(imageFolder + ".srt").c_str(), ofstream::out | ofstream::trunc);
        fout.close();
    }
    else {
        ofstream fout;
        fout.open(ofToDataPath(imageFolder + ".srt").c_str());
        fout.seekp(0, ios::end); // start writing at the end
        fout << ss.str();
        fout.close();
    }
}

void ofApp::applyFilter() {
    int w = imageBuffer.getWidth();
    int h = imageBuffer.getHeight();
    
    int channelCount = imageBuffer.getPixelsRef().getNumChannels();
    
    for (int y = 0; y < h; y++) {
        
        unsigned char * cursor = imageBuffer.getPixels() + ((w * channelCount) * y);
        
        for (int x = 0; x < w - 1; x++) {
            
            double tintPercentage = .25;
            ofColor filterColor = ofColor::green;
            
            // cout << "Before: " << cursor[0] << " " << cursor[1] << " " << cursor[2] << endl;
            
            int a0 = cursor[0];
            int a1 = cursor[1];
            int a2 = cursor[2];
            
            cursor[0] = cursor[0] + (tintPercentage * (filterColor.r - cursor[0]));
            cursor[1] = cursor[1] + (tintPercentage * (filterColor.g - cursor[1]));
            cursor[2] = cursor[2] + (tintPercentage * (filterColor.b - cursor[2]));
            
            // cout << "After: " << cursor[0] << " " << cursor[1] << " " << cursor[2] << endl;
            
            int b0 = cursor[0];
            int b1 = cursor[1];
            int b2 = cursor[2];
            cursor += channelCount;
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
void ofApp::loadLUT(string path){
    LUTloaded = false;
    
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
                
                imageBuffer.setColor(x, y, color);
            }			
        }
        
        imageBuffer.update();
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
        if (dirLoadIndex >= (int)dirLut.size()) {
            dirLoadIndex = 0;
        }
        loadLUT(dirLut.getPath(dirLoadIndex));
    }
    if (key == OF_KEY_DOWN) {
        dirLoadIndex--;
        if (dirLoadIndex < 0) {
            dirLoadIndex = dirLut.size()-1;
        }
        loadLUT(dirLut.getPath(dirLoadIndex));
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
