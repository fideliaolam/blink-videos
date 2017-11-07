#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    dir.allowExt("mov");
    dir.listDir(ofToDataPath("/Users/fidelialam/Documents/USC/605/FinalProject/videos/selected/"));
    currentFileIndex = 0;
    
    videoLoader = true;
    
    vid.load(dir.getPath(currentFileIndex));
    vid.play();
    
    vidPtr = shared_ptr<ofxHAPAVPlayer>(new ofxHAPAVPlayer);
    vidPtr->load(dir.getPath(currentFileIndex));
    vidPtr->play();
    vidPtr->setPosition(0.5);
    vidPtr->setSpeed(2.0);
    vidPtr->setLoopState(OF_LOOP_NONE);

}

//--------------------------------------------------------------
void ofApp::update(){
    vid.update();
    vidPtr->update();
    //if(ofGetFrameNum() % 20 == 0 && bLoadRandom) keyReleased(' ');
    //if (vidPtr->getIsMovieDone()) keyReleased(' ');
}

//--------------------------------------------------------------
void ofApp::draw(){
    vid.draw(0,0, 1280,800);
    //vidPtr->draw(0, 0, 1280,800);
    ostringstream os;
    os << "FPS: " << ofGetFrameRate() << endl;
    os << "Frame/Total: "   << vid.getCurrentFrame() << " / " << vid.getTotalNumFrames() << " "
    //<< vid.getPosition() << " / " << vid.getDuration() << " "
    << vidPtr->getCurrentFrame() << " / " << vidPtr->getTotalNumFrames() << " "
    //<< vidPtr->getPosition() << " / " << vidPtr->getDuration() << " "
    << endl;
    os << "Press ' ' (SpaceBar) to load movies at random" << endl;
    os << "Press 'r' to toggle auto load movies at random" << endl;
    os << "Press 'd' to toggle testing destructor/constructor loading" << endl;
    
    ofDrawBitmapString(os.str(), 20, ofGetHeight() - 80);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case ' ':
        {
            spacePressCount++;
            currentFileIndex++;
            if(currentFileIndex == dir.size()) currentFileIndex = 0;
            vid.load(dir.getPath(currentFileIndex));
            vid.play();
            videoLoader = true;
            
            if(videoLoader){
                vidPtr = shared_ptr<ofxHAPAVPlayer>(new ofxHAPAVPlayer);
                vidPtr->load(dir.getPath(currentFileIndex));
                vidPtr->play();
                vidPtr->setSpeed(2.0);
                videoLoader = false;
            }
            
            cout << "Space pressed " << spacePressCount << endl;
            
            //cout << dir.getPath(currentFileIndex) << endl;
            
        }
            break;
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
