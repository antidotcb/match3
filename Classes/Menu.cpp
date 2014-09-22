#include "Menu.h"

#include <2d/CCMenu.h>
#include <2d/CCMenuItem.h>
#include <2d/CCSprite.h>
#include <2d/CCTransition.h>
#include <base/ccMacros.h>
#include <base/CCDirector.h>
#include <base/CCPlatformConfig.h>
#include <math/CCGeometry.h>
#include <math/Vec2.h>

#include "AppDelegate.h"
#include "Game.h"

namespace match3 {
    USING_NS_CC;

    cocos2d::Scene* MenuLayer::wrapIntoScene()
            {
        auto scene = Scene::create();
        scene->addChild(MenuLayer::create());
        return scene;
    }

// on "init" you need to initialize your instance
    bool MenuLayer::init() {
        //////////////////////////////
        // 1. super init first
        if (!Layer::init()) {
            return false;
        }

        Size visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        auto logo = Sprite::create("logo.png");
        logo->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height + origin.y - logo->getContentSize().height));
        this->addChild(logo, 10);

        MenuItemFont::setFontName("fonts/Marker Felt.ttf");
        auto item1 = MenuItemFont::create("Play",
                CC_CALLBACK_1(MenuLayer::menuPlay, this));
        auto item2 = MenuItemFont::create("Highscores",
                CC_CALLBACK_1(MenuLayer::menuHighscores, this));
        auto item3 = MenuItemFont::create("Settings",
                CC_CALLBACK_1(MenuLayer::menuSettings, this));
        auto item4 = MenuItemImage::create("exit_x48.png", "exit_pressed_x48.png",
                CC_CALLBACK_1(MenuLayer::menuClose, this));

        auto menu = Menu::create(item1, item2, item3, item4, nullptr);
        menu->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y - logo->getContentSize().height));
        menu->alignItemsVertically();
        addChild(menu, 5);

        auto background = Sprite::create("bg3.png");
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(background, 0);

        return true;
    }

    void MenuLayer::menuPlay(Ref* pSender) {
        auto game = GameLayer::wrapIntoScene();
        auto transition = TransitionZoomFlipX::create(1, game);
        Director::getInstance()->pushScene(transition);
    }

    void MenuLayer::menuHighscores(Ref* pSender) {
        //Director::getInstance()->end();
    }

    void MenuLayer::menuSettings(Ref* pSender) {
        //Director::getInstance()->end();
    }

    void MenuLayer::menuClose(Ref* pSender) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
        MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
        return;
#endif
        Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        exit(0);
#endif
    }

} /* namespace match3 */
