#include "AppDelegate.h"

#include <base/CCDirector.h>
#include <base/CCPlatformMacros.h>
#include <CCGLView.h>
#include <CCGLViewProtocol.h>
#include <math/CCGeometry.h>

#include "HelloWorldScene.h"
#include "Piece.h"

USING_NS_CC;

using namespace match3;

AppDelegate::AppDelegate() {
    PiecesManager::getInstance();
}

AppDelegate::~AppDelegate() {
    PiecesManager::destroyInstance();
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if (!glview) {
        glview = GLView::create("match3");
        glview->setFrameSize(800, 600);
        director->setOpenGLView(glview);
        setupResolutionPolicy(800, 600);
    }

    // turn on display FPS
    // director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    // load images
    PiecesManager::getInstance()->loadPieces("colors.txt");

    // create a scene. it's an autorelease object
    auto scene = HelloWorld::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

void AppDelegate::setupResolutionPolicy(float designW, float designH) {
    Size screenSize =  Director::getInstance()->getOpenGLView()->getFrameSize();

    float designRatio = designW / designH;
    float screenRatio = screenSize.height / screenSize.width;

    ResolutionPolicy resolutionPolicy = screenRatio < designRatio ?
                                                                    ResolutionPolicy::FIXED_HEIGHT :
                                                                    ResolutionPolicy::FIXED_WIDTH;

    Director::getInstance()->getOpenGLView()->setDesignResolutionSize(designW, designH, resolutionPolicy);
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
