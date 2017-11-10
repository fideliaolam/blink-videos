#include "ofApp.h"

GLdouble modelviewMatrix[16], projectionMatrix[16];
GLint viewport[4];
void updateProjectionState() {
    glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
}

ofVec3f ofWorldToScreen(ofVec3f world) {
    updateProjectionState();
    GLdouble x, y, z;
    gluProject(world.x, world.y, world.z, modelviewMatrix, projectionMatrix, viewport, &x, &y, &z);
    ofVec3f screen(x, y, z);
    screen.y = ofGetHeight() - screen.y;
    return screen;
}

ofMesh getProjectedMesh(const ofMesh& mesh) {
    ofMesh projected = mesh;
    for(int i = 0; i < mesh.getNumVertices(); i++) {
        ofVec3f cur = ofWorldToScreen(mesh.getVerticesPointer()[i]);
        cur.z = 0;
        projected.setVertex(i, cur);
    }
    return projected;
}

template <class T>
void addTexCoords(ofMesh& to, const vector<T>& from) {
    for(int i = 0; i < from.size(); i++) {
        to.addTexCoord(from[i]);
    }
}

using namespace ofxCv;

void ofApp::setupCamTrack(){
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
    cam.initGrabber(640,480);
    
    // Set up images we'll use for frame-differencing
    camImage.allocate(640, 480, OF_IMAGE_COLOR);
    eyeImageColor.allocate(128, 48);
    eyeImageGray.allocate(128,48);
    eyeImageGrayPrev.allocate(128,48);
    eyeImageGrayDif.allocate(128,48);
    eyeFbo.allocate(128, 48, GL_RGB);
    
    // Initialize our images to black.
    eyeImageColor.set(0);
    eyeImageGray.set(0);
    eyeImageGrayPrev.set(0);
    eyeImageGrayDif.set(0);
    
    // Set up other objects.
    tracker.setup();
    osc.setup("localhost", 8338);
    
    // This GUI slider adjusts the sensitivity of the blink detector.
    gui.setup();
    gui.add(percentileThreshold.setup("Threshold", 0.967, 0.75, 1.0));

}

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
    cam.update();
    vid.update();
    vidPtr->update();
    //if(ofGetFrameNum() % 20 == 0 && bLoadRandom) keyReleased(' ');
    //if (vidPtr->getIsMovieDone()) keyReleased(' ');
    
    if(cam.isFrameNew()) {
        camImage.setFromPixels(cam.getPixels());
        
        tracker.update(toCv(cam));
        position = tracker.getPosition();
        scale = tracker.getScale();
        rotationMatrix = tracker.getRotationMatrix();
        
        if(tracker.getFound()) {
            //not entering this
            ofVec2f
            leftOuter  = tracker.getImagePoint(36),
            leftInner  = tracker.getImagePoint(39),
            rightInner = tracker.getImagePoint(42),
            rightOuter = tracker.getImagePoint(45);
            
            ofPolyline leftEye = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE);
            ofPolyline rightEye = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE);
            
            ofVec2f leftCenter = leftEye.getBoundingBox().getCenter();
            ofVec2f rightCenter = rightEye.getBoundingBox().getCenter();
            
            float leftRadius = (leftCenter.distance(leftInner) + leftCenter.distance(leftOuter)) / 2;
            float rightRadius = (rightCenter.distance(rightInner) + rightCenter.distance(rightOuter)) / 2;
            
            ofVec2f
            leftOuterObj  = tracker.getObjectPoint(36),
            leftInnerObj  = tracker.getObjectPoint(39),
            rightInnerObj = tracker.getObjectPoint(42),
            rightOuterObj = tracker.getObjectPoint(45);
            
            ofVec3f upperBorder(0, -3.5, 0), lowerBorder(0, 2.5, 0);
            ofVec3f leftDirection(-1, 0, 0), rightDirection(+1, 0, 0);
            float innerBorder = 1.5, outerBorder = 2.5;
            
            ofMesh leftRect, rightRect;
            leftRect.setMode(OF_PRIMITIVE_LINE_LOOP);
            leftRect.addVertex(leftOuterObj + upperBorder + leftDirection * outerBorder);
            leftRect.addVertex(leftInnerObj + upperBorder + rightDirection * innerBorder);
            leftRect.addVertex(leftInnerObj + lowerBorder + rightDirection * innerBorder);
            leftRect.addVertex(leftOuterObj + lowerBorder + leftDirection * outerBorder);
            rightRect.setMode(OF_PRIMITIVE_LINE_LOOP);
            rightRect.addVertex(rightInnerObj+ upperBorder + leftDirection * innerBorder);
            rightRect.addVertex(rightOuterObj + upperBorder + rightDirection * outerBorder);
            rightRect.addVertex(rightOuterObj + lowerBorder + rightDirection * outerBorder);
            rightRect.addVertex(rightInnerObj + lowerBorder + leftDirection * innerBorder);
            
            ofPushMatrix();
            ofSetupScreenOrtho(640, 480, -1000, 1000);
            ofScale(1, 1, -1);
            ofTranslate(position);
            applyMatrix(rotationMatrix);
            ofScale(scale, scale, scale);
            leftRectImg = getProjectedMesh(leftRect);
            rightRectImg = getProjectedMesh(rightRect);
            ofPopMatrix();
            
            // more effective than using object space points would be to use image space
            // but translate to the center of the eye and orient the rectangle in the
            // direction the face is facing.
            /*
             ofPushMatrix();
             ofTranslate(tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).getCentroid2D());
             applyMatrix(rotationMatrix);
             ofRect(-50, -40, 2*50, 2*40);
             ofPopMatrix();
             
             ofPushMatrix();
             ofTranslate(tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).getCentroid2D());
             applyMatrix(rotationMatrix);
             ofRect(-50, -40, 2*50, 2*40);
             ofPopMatrix();
             */
            
            ofMesh normRect, normLeft, normRight;
            normRect.addVertex(ofVec2f(0, 0));
            normRect.addVertex(ofVec2f(64, 0));
            normRect.addVertex(ofVec2f(64, 48));
            normRect.addVertex(ofVec2f(0, 48));
            normLeft.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
            normRight.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
            normLeft.addVertices(normRect.getVertices());
            normRight.addVertices(normRect.getVertices());
            addTexCoords(normLeft, leftRectImg.getVertices());
            addTexCoords(normRight, rightRectImg.getVertices());
            
            // Copy the extracted quadrilaterals into the eyeFbo
            eyeFbo.begin();
            ofSetColor(255);
            ofFill();
            cam.getTexture().bind();
            normLeft.draw();
            ofTranslate(64, 0);
            normRight.draw();
            cam.getTexture().unbind();
            eyeFbo.end();
            eyeFbo.readToPixels(eyePixels);
            
            // Convert the combined eye-image to grayscale,
            // and put its data into a frame-differencer.
            eyeImageColor.setFromPixels(eyePixels);
            eyeImageGrayPrev.setFromPixels(eyeImageGray.getPixels());
            eyeImageGray.setFromColorImage(eyeImageColor);
            eyeImageGray.contrastStretch();
            eyeImageGrayDif.absDiff(eyeImageGrayPrev, eyeImageGray);
            
            // Compute the average brightness of the difference image.
            unsigned char* difPixels = eyeImageGrayDif.getPixels().getData();
            int nPixels = 128*48;
            float sum = 0;
            for (int i=0; i<nPixels; i++){
                if (difPixels[i] > 4){ // don't consider diffs less than 4 gray levels;
                    sum += difPixels[i]; // reduces noise
                }
            }
            // Store the current average in the row graph
            float avg = sum / (float) nPixels;
            rowGraph.addSample(avg, percentileThreshold);
            
            // Send out an OSC message,
            // With the value 1 if the current average is above threshold
            ofxOscMessage msg;
            msg.setAddress("/gesture/blink");
            int oscMsgInt = (rowGraph.getState() ? 1 : 0);
            msg.addIntArg(oscMsgInt);
            osc.sendMessage(msg);
            
            // Print out a message to the console if there was a blink.
            if (oscMsgInt > 0){
                printf("Blink happened at frame #:	%d\n", (int)ofGetFrameNum());
                keyPressed(' ');
            }
            
        }
    }

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

void ofApp::drawCamWindow(ofEventArgs & args){
    camImage.resize(640, 480);
    camImage.draw(0,0);
    tracker.draw();
    leftRectImg.draw();
    rightRectImg.draw();
    
    float y = 58;
    gui.draw();
    eyeImageGray.draw(10, y);    y+=58;
    eyeImageGrayDif.draw(10, y); y+=58;
    rowGraph.draw(10, y, 48);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case ' ':
        {
            spacePressCount++;
            currentFileIndex = rand() % dir.size();
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
        case 'r':
        {
            tracker.reset();
        }
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
