#include "AppDelegate.h"

#include <2d/CCScene.h>
#include <base/CCDirector.h>
#include <base/CCPlatformMacros.h>
#include <CCGLView.h>
#include <CCGLViewProtocol.h>
#include <math/CCGeometry.h>
#include <SimpleAudioEngine.h>

#include "Menu.h"
#include "Piece.h"

namespace match3 {
    USING_NS_CC;

    Scene* App::wrapIntoScene(Layer* _Layer) {
        auto scene = Scene::create();
        scene->addChild(_Layer);
        return scene;
    }

    App::App() {
        PiecesManager::getInstance();
    }

    App::~App() {
        PiecesManager::destroyInstance();
    }

    bool App::applicationDidFinishLaunching() {
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
        PiecesManager::getInstance()->loadTextures();
        auto scene = wrapIntoScene(MenuLayer::create());
        director->runWithScene(scene);

        return true;
    }

    void App::adaptResolution(float _DesignW, float _DesignH) {
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

    void App::applicationDidEnterBackground() {
        Director::getInstance()->stopAnimation();
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    }

    void App::applicationWillEnterForeground() {
        Director::getInstance()->startAnimation();
        CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    }

} /* namespace match3 */
