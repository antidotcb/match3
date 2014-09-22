#include "AppDelegate.h"

#include <SimpleAudioEngine.h>

#include "Menu.h"
#include "Piece.h"


    USING_NS_CC;


    AppDelegate::AppDelegate() {
        match3::PiecesManager::getInstance();
    }

    AppDelegate::~AppDelegate() {
        match3::PiecesManager::destroyInstance();
    }

    bool AppDelegate::applicationDidFinishLaunching() {
        auto director = Director::getInstance();
        auto glview = director->getOpenGLView();
        if (!glview) {
            glview = GLView::create("match3");
            director->setOpenGLView(glview);
            adaptResolution(640, 480);
        }

        // turn on display FPS
        director->setDisplayStats(false);

        director->setAnimationInterval(1.0 / 60);
        match3::PiecesManager::getInstance()->loadTextures();
        auto scene = match3::MenuLayer::wrapIntoScene();
        director->runWithScene(scene);

        return true;
    }

    void AppDelegate::adaptResolution(float _DesignW, float _DesignH) {
        auto director = Director::getInstance();
        Size screenSize = director->getOpenGLView()->getFrameSize();

        const float designRatio = _DesignW / _DesignH;
        const float screenRatio = screenSize.height / screenSize.width;

        ResolutionPolicy resolutionPolicy = ResolutionPolicy::FIXED_WIDTH;
        if (screenRatio < designRatio) {
            resolutionPolicy = ResolutionPolicy::FIXED_HEIGHT;
        }

        director->getOpenGLView()->setDesignResolutionSize(_DesignW, _DesignH, resolutionPolicy);
    }

    void AppDelegate::applicationDidEnterBackground() {
        Director::getInstance()->stopAnimation();
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    }

    void AppDelegate::applicationWillEnterForeground() {
        Director::getInstance()->startAnimation();
        CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    }


