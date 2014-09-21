#include "Game.h"

#include <cocos2d.h>
#include <cstdint>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "Board.h"
#include "GameboardLayer.h"
#include "Piece.h"
#include "Score.h"

USING_NS_CC;

namespace match3 {
    const float GameLayer::TotalGameTime = 108;

    const int GameLayer::DefaultBoardSize = 8;
    const int GameLayer::BackgroundLayerLevel = -100;

    const char* GameLayer::BackgroundTextureName = "bg3.png";

    bool GameLayer::init() {
        if (!Layer::init()) {
            return false;
        }

        CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music.mp3");
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("move.aif");

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

        addChild(scoreLabel_);

        score_ = new Score();
        score_->setLabel(scoreLabel_);

        addChild(score_);
    }

    void GameLayer::addGameboard() {
        Gameboard::Size boardSize = { DefaultBoardSize, DefaultBoardSize };
        Vec2 pos(visibleSize_.width / 2 + origin_.x,
                visibleSize_.height / 2 + origin_.y);

        gameboard_ = Gameboard::create(boardSize);

        gameboardlayer = GameBoardLayer::create(gameboard_, score_);
        gameboardlayer->setPosition(pos);
        addChild(gameboardlayer);
    }

    void GameLayer::addInputDispatcher()
    {
        EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();

        listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
        listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);

        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    }

    bool GameLayer::onTouchBegan(cocos2d::Touch *_Touch, cocos2d::Event *_Event) {
        if (gameEnded_) {
            // no any processing if game ended
            return true;
        } else {
            return gameboardlayer->onTouchBegan(_Touch, _Event);
        }
    }

    void GameLayer::onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event) {
        if (gameEnded_) {
            return;
        } else {
            gameboardlayer->onTouchMoved(_Touch, _Event);
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
        //gameboard_->lock();

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

        if (!turnCompleted_) {
            auto waitTillTurnCompleted = Sequence::create(
                    DelayTime::create(0.5f),
                    CallFunc::create(CC_CALLBACK_0(GameLayer::onTimeExpires, this)),
                    nullptr);

            runAction(waitTillTurnCompleted);
            return;
        }

        //gameboard_->removeFromParentAndCleanup(true);

        CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("groovy.mp3");

        auto endThisGame = Sequence::create(
                DelayTime::create(2.f),
                CallFunc::create(CC_CALLBACK_0(GameLayer::endGame, this)),
                nullptr);

        runAction(endThisGame);

        CCLOG("Time is out");
    }

    void GameLayer::startTurn() {
        turnCompleted_ = false;
        //deselect();
        //gameboard_->lock();
        score_->newTurn();
    }

    void GameLayer::endTurn() {
        if (!gameEnded_) {
            //gameboard_->unlock();
        }
        turnCompleted_ = true;
    }

    void GameLayer::endGame() {
        if (highscore_) {
            // TODO: add some
            //Director::
        } else {
            Director::getInstance()->popToRootScene();
        }
    }

    void GameLayer::addProgressTimer() {
        timer_ = ProgressTimer::create(Sprite::create("hor_progress_yellow.png"));

        timer_->setPosition(origin_.x + visibleSize_.width / 2.f, origin_.y + 10);

        timer_->setType(ProgressTimerType::BAR);
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

    void GameLayer::cleanup()
    {
        if (gameboard_) {
            delete gameboard_;
            gameboard_ = nullptr;
        }
        Layer::cleanup();
    }

}
