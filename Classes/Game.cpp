#include "Game.h"

#include <2d/CCActionInstant.h>
#include <2d/CCActionInterval.h>
#include <2d/CCLabel.h>
#include <2d/CCParticleExamples.h>
#include <2d/CCProgressTimer.h>
#include <2d/CCSprite.h>
#include <2d/CCTransition.h>
#include <2d/CCTransitionPageTurn.h>
#include <base/ccMacros.h>
#include <base/ccTypes.h>
#include <base/CCDirector.h>
#include <base/CCEventDispatcher.h>
#include <base/CCEventListenerTouch.h>
#include <renderer/CCTexture2D.h>
#include <renderer/CCTextureCache.h>
#include <SimpleAudioEngine.h>
#include <cstdint>

#include "AppDelegate.h"
#include "Gameboard.h"
#include "Menu.h"

USING_NS_CC;

namespace match3 {
    const float GameLayer::TotalGameTime = 8;
    const int GameLayer::BackgroundLayerLevel = -100;

    const char* GameLayer::BackgroundTextureName = "bg3.png";

    cocos2d::Scene* GameLayer::wrapIntoScene()
            {
        auto scene = Scene::create();
        scene->addChild(GameLayer::create());
        return scene;
    }

    bool GameLayer::init() {
        if (!Layer::init()) {
            return false;
        }

        CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music.mp3");
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("move.aif");
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("remove.wav");
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("groovy.mp3");

        visibleSize_ = Director::getInstance()->getVisibleSize();
        origin_ = Director::getInstance()->getVisibleOrigin();

        addInputDispatcher();
        addScores();
        addGameboard();
        addBackground();
        addProgressTimer();

        //addUIElements();

        CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("music.mp3", true);
        return true;
    }

    void GameLayer::addBackground()
    {
        Sprite* sprite = Sprite::create(BackgroundTextureName);
        sprite->setPosition(Vec2(visibleSize_.width / 2 + origin_.x,
                visibleSize_.height / 2 + origin_.y));
        addChild(sprite, BackgroundLayerLevel);

        auto backgroundPS = ParticleSnow::createWithTotalParticles(200);
        auto texture = Director::getInstance()->getTextureCache()->addImage("stars2.png");
        auto rect = Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height);
        backgroundPS->setTextureWithRect(texture, rect);

        addChild(backgroundPS, BackgroundLayerLevel + 1);
    }

    void GameLayer::addScores()
    {
        scoreLabel_ = Label::createWithBMFont("fonts/font.fnt", "0", TextHAlignment::CENTER);
        scoreLabel_->setPosition(Vec2(visibleSize_.width / 2 + origin_.x,
                visibleSize_.height - scoreLabel_->getContentSize().height - 2));
        addChild(scoreLabel_, UILayerLevel);

        score_.setLabel(scoreLabel_);
    }

    void GameLayer::addGameboard() {
        Vec2 pos(visibleSize_.width / 2 + origin_.x,
                visibleSize_.height / 2 + origin_.y);

        gameboard_ = Gameboard::create(&score_);
        gameboard_->setPosition(pos);

        addChild(gameboard_, 0);
    }

    void GameLayer::addInputDispatcher()
    {
        EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();

        listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
        listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);

        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    }

    GameLayer::~GameLayer()
    {
        CocosDenshion::SimpleAudioEngine::getInstance()->unloadEffect("music.mp3");
        CocosDenshion::SimpleAudioEngine::getInstance()->unloadEffect("move.aif");
        CocosDenshion::SimpleAudioEngine::getInstance()->unloadEffect("remove.wav");
        CocosDenshion::SimpleAudioEngine::getInstance()->unloadEffect("groovy.mp3");
    }

    bool GameLayer::onTouchBegan(cocos2d::Touch *_Touch, cocos2d::Event *_Event) {
        if (gameEnded_) {
            if (highscore_) {
                if (gameboard_ && !gameboard_->isActive()) {
                    return true;
                }
                auto transition = TransitionPageTurn::create(2.f, MenuLayer::wrapIntoScene(), false);
                Director::getInstance()->replaceScene(transition);
            }
            return true;
        } else {
            return gameboard_->onTouchBegan(_Touch, _Event);
        }
    }

    void GameLayer::onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event) {
        if (gameEnded_) {
            return;
        } else {
            gameboard_->onTouchMoved(_Touch, _Event);
        }
    }

    void GameLayer::onProgressTimer() {
        time += delayTime;
        float percent = 100 - (time / TotalGameTime) * 100;
        timer_->setPercentage(percent);
    }

    void GameLayer::onTimeExpires() {
        CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic(true);
        gameEnded_ = true;

        if (!enlargedScoreOnce_) {
            auto center = Vec2(origin_.x + visibleSize_.width / 2.f,
                    origin_.y + visibleSize_.height / 2);
            auto centerEnlarge = Spawn::create(
                    MoveTo::create(2.f, center),
                    ScaleTo::create(2.f, 3.0f),
                    nullptr);

            scoreLabel_->runAction(centerEnlarge);
            enlargedScoreOnce_ = true;
        }

        if (gameboard_->isActive()) {
            auto waitTillTurnCompleted = Sequence::create(
                    DelayTime::create(0.5f),
                    CallFunc::create(CC_CALLBACK_0(GameLayer::onTimeExpires, this)),
                    nullptr);

            runAction(waitTillTurnCompleted);
            return;
        }

        gameboard_->removeFromParentAndCleanup(true);
        gameboard_ = nullptr;

        CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();

        auto endThisGame = Sequence::create(
                DelayTime::create(2.f),
                CallFunc::create(CC_CALLBACK_0(GameLayer::endGame, this)),
                nullptr);

        runAction(endThisGame);

        CCLOG("Time is out");
    }

    void GameLayer::endGame() {
        static uint32_t maxScore = 0;
        highscore_ = (score_.score() > maxScore);
        if (highscore_) {
            maxScore = score_.score();
            Label* label = Label::createWithSystemFont("Highscore!!!", "Arial Bold", 36);
            label->enableShadow();
            label->setPosition(Vec2(origin_.x + visibleSize_.width / 2.f, origin_.y + visibleSize_.height - label->getContentSize().height - 10));
            addChild(label, UILayerLevel);
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("groovy.mp3");

            auto firework = ParticleFireworks::createWithTotalParticles(500);
            firework->setPosition(Vec2(origin_.x + visibleSize_.width / 2.f, origin_.y + 10));
            addChild(firework, UILayerLevel - 1);
            // wait mouse click to exit
        } else {
            auto transition = TransitionRotoZoom::create(2.f, MenuLayer::wrapIntoScene());
            Director::getInstance()->replaceScene(transition);
        }
    }

    void GameLayer::addProgressTimer() {
        timer_ = ProgressTimer::create(Sprite::create("hor_progress_yellow.png"));

        timer_->setPosition(origin_.x + visibleSize_.width / 2.f, origin_.y + 10);

        timer_->setType(ProgressTimer::Type::BAR);
        timer_->setMidpoint( { 0, -1 });
        timer_->setBarChangeRate( { 1, 0 });
        timer_->setScale(visibleSize_.width, 0.5f);

        timer_->setPercentage(100);
        addChild(timer_, UILayerLevel);

        delayTime = 1.0f / (visibleSize_.width / TotalGameTime);
        CCLOG("Progress bar update freq: %f", delayTime);

        auto time_seq = Sequence::create(
                DelayTime::create(delayTime),
                CallFunc::create(CC_CALLBACK_0(GameLayer::onProgressTimer, this)),
                nullptr);

        auto final_seq = Sequence::create(
                Repeat::create(time_seq, TotalGameTime / delayTime),
                CallFunc::create(CC_CALLBACK_0(GameLayer::onTimeExpires, this)),
                nullptr);

        runAction(final_seq);
    }

}
