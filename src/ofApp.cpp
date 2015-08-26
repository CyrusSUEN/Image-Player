#include "ofApp.h"

void ofApp::setup() {
    
    // hide mouse cursor
    ofHideCursor();
    
    ofBackground(0);
    ofSetWindowTitle("Image Player");
    
    // load sound
    vocal.loadSound("sound/audio.mp3");
    
    dirImg.allowExt("jpg");
    // load the first image
    dynamicLoading(0);
    
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

void ofApp::dynamicLoading(int i) {
    if (dirImg.listDir("frames") > i) {
        loader.loadFromDisk(imageBuffer, dirImg.getPath(i));
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

void ofApp::displaySubtitle(int imagesIndex, int frameNum) {
    static int frameCount = 0;
    static int frameLasts = 0;
    static int playedImagesIndex = -1;
    
    static stringstream ss;
    static float offsetF;
    static float offsetM;
    
    if (playedImagesIndex == -1 && frameCount == 0 && frameLasts == 0) {
        offsetF = ofGetElapsedTimef();
        offsetM = ofGetElapsedTimeMillis();
    }
    
    if (imagesIndex > playedImagesIndex + 1) {
        frameCount = 0;
        frameLasts = 0;
        playedImagesIndex++;
        
        {
            ss << " --> ";
            getElapsedTime(ss, offsetF, offsetM);
            ss << endl << json[playedImagesIndex]["text"].asString() << endl << endl;
            
            // ofLogNotice("Subtitle") << ss.str();
            
            writeSrtFile(ss);
            
            // ss.str(""); // clear the content
        }
    }
    
    if (playedImagesIndex < imagesIndex) {
        if (frameCount == 0) {
            if (json[imagesIndex]["index"] == imagesIndex) {
                frameLasts = json[imagesIndex]["frames"].asInt();
                
                {
                    ss << imagesIndex + 1 << endl;
                    getElapsedTime(ss, offsetF, offsetM);
                }

            }
        }
        
        if (frameCount <= frameLasts) {
            ofDrawBitmapString(json[imagesIndex]["text"].asString(), 15, 20);
            frameCount++;
        }
        else {
            frameCount = 0;
            frameLasts = 0;
            playedImagesIndex++;
            
            {
                ss << " --> ";
                getElapsedTime(ss, offsetF, offsetM);
                ss << endl << json[playedImagesIndex]["text"].asString() << endl << endl;
                
                ofLogNotice("Subtitle") << ss.str();
                
                writeSrtFile(ss);
                
                // ss.str(""); // clear the content
            }
        }
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

void ofApp::writeSrtFile(const stringstream& ss) {
    ofstream fout;
    fout.open(ofToDataPath("a.srt").c_str());
    fout << ss.str();
    fout.close();
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
            dynamicLoading(imagesIndex);
            
            imagesIndex++;
            
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
            displaySubtitle(imagesIndex - 1, frameNum);
        }
        else {
            // draw the image sequence at the new frame count
            imageBuffer.draw(256, 36);
            
            // how fast is the app running and some other info
            ofSetColor(50);
            ofRect(0, 0, 200, 75);
            ofSetColor(200);
            string info;
            info += ofToString(frameIndex) + "/" + ofToString(dirImg.size()-1) + " file index\n";
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
