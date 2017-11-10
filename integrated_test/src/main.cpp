#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings window;
    window.width = 1200;
    window.height = 800;
    window.setPosition (ofVec2f(200,200));
    window.resizable = true;
    
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(window);
    
    window.width = 640;
    window.height = 480;
    window.setPosition(ofVec2f(0,0));
    window.resizable = false;
    
    window.shareContextWith = mainWindow;
    shared_ptr<ofAppBaseWindow> camWindow  = ofCreateWindow(window);
    camWindow->setVerticalSync(true);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    mainApp->setupCamTrack();
    ofAddListener(camWindow->events().draw, mainApp.get(), &ofApp::drawCamWindow);
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();

}
