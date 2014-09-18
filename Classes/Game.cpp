#include "Game.h"

#include <2d/CCAction.h>
#include <2d/CCActionInstant.h>
#include <2d/CCActionInterval.h>
#include <2d/CCLabelTTF.h>
#include <2d/CCScene.h>
#include <2d/CCSprite.h>
#include <base/ccMacros.h>
#include <base/ccTypes.h>
#include <base/CCDirector.h>
#include <base/CCEventDispatcher.h>
#include <base/CCEventListenerTouch.h>
#include <base/CCPlatformMacros.h>
#include <base/CCTouch.h>
#include <math/CCGeometry.h>
#include <math/Vec2.h>
#include <sys/types.h>
#include <SimpleAudioEngine.h>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "Board.h"
#include "Piece.h"

USING_NS_CC;

namespace match3 {
    const int GameLayer::DefaultBoardSize = 8;
    const int GameLayer::BackgroundLayerLevel = -100;
    const int GameLayer::HighlightActionsTag = 50;

    const float GameLayer::FastSpeed = .25f;
    const float GameLayer::SlowSpeed = .5f;
    const float GameLayer::DissapearSpeed = .1f;
    //const float Gameboard::SlowSpeed = .25f;

    const float GameLayer::FastSpeedLL = GameLayer::FastSpeed + (GameLayer::FastSpeed / 10.0f);
    const float GameLayer::SlowSpeedLL = GameLayer::SlowSpeed + (GameLayer::SlowSpeed / 10.0f);

    const char* GameLayer::BackgroundTextureName = "bg.png";

    Scene* GameLayer::wrapIntoScene() {
        auto scene = Scene::create();
        auto layer = GameLayer::create();
        scene->addChild(layer);
        return scene;
    }

    GameLayer::GameLayer() :
            gameboard_(0), selected_(0), firstArrived_(0) {
        visibleSize_ = Size { 0, 0 };
        origin_ = Vec2(0, 0);
    }

    bool GameLayer::init() {
        if (!Layer::init()) {
            return false;
        }

        CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music.mp3");
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("move.aif");

        visibleSize_ = Director::getInstance()->getVisibleSize();
        origin_ = Director::getInstance()->getVisibleOrigin();

        addInputDispatcher();
        addGameboard();
        addBackground();

        /*
         //if (!addGameboard()) {
         // TODO:
         //   CCLOGERROR("Can't init gameboard.");
         //   return false;
         //}
         //CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("music.mp3", true);
         * */
        return true;
    }

    void GameLayer::addBackground()
    {
        Sprite* sprite = Sprite::create(BackgroundTextureName);
        sprite->setPosition(Vec2(visibleSize_.width / 2 + origin_.x,
                visibleSize_.height / 2 + origin_.y));
        this->addChild(sprite, BackgroundLayerLevel);
    }

    bool GameLayer::addGameboard() {
        Gameboard::Size boardSize = { DefaultBoardSize, DefaultBoardSize };
        Vec2 position;
        gameboard_ = Gameboard::create(position, boardSize, PiecesManager::getInstance(), this);

        if (!gameboard_) {
            CCLOGERROR("Can't create gameboard.");
            return false;
        }

        return true;
    }

    void GameLayer::addInputDispatcher()
    {
        EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();

        listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
        listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);

        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    }

    bool GameLayer::onTouchBegan(cocos2d::Touch *_Touch, cocos2d::Event *_Event) {
        Vec2 touch_pos = _Touch->getLocation();
        Coord coord = gameboard_->screenToCell(touch_pos);
        Piece* piece = gameboard_->getPiece(coord);

        if (gameboard_->locked() || !piece) {
            return true;
        }

        CCLOG("Piece [X=%d , Y=%d], Color: %s", coord.X, coord.Y, piece->color()->name().c_str());

        if (selected_ && selected_->isNextTo(piece)) {
            swapPieces(selected_, piece);
        } else {
            select(piece);
        }

        return true;
    }

    void GameLayer::onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event) {
        Vec2 touch_pos = _Touch->getLocation();
        Coord coord = gameboard_->screenToCell(touch_pos);
        Piece* piece = gameboard_->getPiece(coord);

        if (gameboard_->locked() || !piece) {
            return;
        }

        CCLOG("Piece [X=%d , Y=%d], Color: %s", coord.X, coord.Y, piece->color()->name().c_str());

        if (selected_ && selected_->isNextTo(piece)) {
            swapPieces(selected_, piece);
        }
    }

    void GameLayer::select(Piece* _Piece) {
        if (selected_) {
            deselect();
        }

        selected_ = _Piece;

        auto selectSequence = Sequence::create(
                FadeTo::create(SlowSpeed, 192),
                FadeTo::create(SlowSpeed, 255),
                nullptr);

        auto pulsation = RepeatForever::create(selectSequence);
        pulsation->setTag(HighlightActionsTag);

        selected_->sprite()->runAction(pulsation);
    }

    void GameLayer::deselect() {
        if (!selected_) {
            return;
        }

        selected_->sprite()->stopActionByTag(HighlightActionsTag);
        selected_->sprite()->runAction(FadeTo::create(.0f, 255));
        selected_ = nullptr;
    }

    void GameLayer::afterSwap(Node* _Node) {
        if (firstArrived_ == nullptr) {
            CCLOG("checkSwapValid: First arrived, proceed further.");
            firstArrived_ = _Node;
            return;
        }

        firstArrived_ = nullptr;

        CCLOG("checkSwapValid: Second arrived, finally can check.");

        std::list<std::vector<Piece*> > removePieces;
        gameboard_->getResultsOfLastCheck(removePieces);

        uint score = 0;
        uint group_bonus = 0;
        const uint pieceScore = 10;

        for (auto vec : removePieces) {
            CCLOG("Found %d pieces in row", vec.size());
            group_bonus += 5;
            for (Piece* piece : vec) {
                gameboard_->remove(piece);

                uint scoreValue = pieceScore * vec.size() + group_bonus;
                score += scoreValue;

                addScoreLabel(scoreValue, piece->sprite()->getPosition());

                dissapear(piece);
            }

            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("remove.wav");
        }

        float longestAnimationTime = gameboard_->fillup();
        float waitFalldownToComplete = longestAnimationTime + FastSpeed;

        if (!gameboard_->check()) {
            auto waitToUnlock = Sequence::create(
                    DelayTime::create(waitFalldownToComplete),
                    CallFunc::create(CC_CALLBACK_0(Gameboard::unlock, gameboard_)),
                    nullptr);
            runAction(waitToUnlock);
        } else {
            std::function<void(Node*)> cb;
            cb = CC_CALLBACK_1(GameLayer::afterSwap, this);

            // Hack - need to call it twice to make it do the job
            auto waitToRecheck = Sequence::create(
                    DelayTime::create(waitFalldownToComplete ),
                    CallFuncN::create(cb),
                    CallFuncN::create(cb),
                    nullptr);

            runAction(waitToRecheck);
        }
    }

    void ScoreToString(uint _Score, std::string& _Str) {
        std::stringstream ss;
        ss << _Score;
        ss >> _Str;
    }

    void GameLayer::addScoreLabel(uint _Score, const Vec2 & _Position) {
        std::string scoreText;
        ScoreToString(_Score, scoreText);
        CCLOG("Score %d == %s", _Score, scoreText.c_str());

        auto label = Label::createWithSystemFont(scoreText, "Impact", 18);

        label->setColor(Color3B::YELLOW);
        label->enableShadow(Color4B::BLACK, Size(2.f, 2.f), 5);
        //label->enableOutline(Color4B::RED, 1);

        addChild(label, ScoreLabelsLayerLevel);

        label->setPosition(_Position);

        auto dissapear = Spawn::create(
                MoveBy::create(SlowSpeed, Vec2(0, 20)),
                FadeTo::create(SlowSpeed, 0.2f),
                ScaleBy::create(SlowSpeed, 0.3f),
                nullptr);

        auto seq = Sequence::create(
                DelayTime::create(DissapearSpeed),
                dissapear,
                RemoveSelf::create(true),
                nullptr);

        label->runAction(seq);
    }

    void GameLayer::afterSwapBack(Node* _Node) {
        if (firstArrived_ == nullptr) {
            CCLOG("swapBack: First arrived, proceed further.");
            firstArrived_ = _Node;
            return;
        }

        firstArrived_ = nullptr;

        gameboard_->unlock();
        CCLOG("swapBack: Second arrived, finally can unlock board.");
    }

    void GameLayer::swapPieces(Piece* _First, Piece* _Second) {
        std::function<void(Node*)> cb;

        gameboard_->lock();
        deselect();

        Sprite* spriteA = _First->sprite();
        Sprite* spriteB = _Second->sprite();

        Coord posA = _First->position();
        Coord posB = _Second->position();

        auto thereA = MoveTo::create(FastSpeed, spriteB->getPosition());
        auto thereB = MoveTo::create(FastSpeed, spriteA->getPosition());

        gameboard_->swap(posA, posB);

        if (gameboard_->check()) {
            cb = CC_CALLBACK_1(GameLayer::afterSwap, this);

            auto callbackA = CallFuncN::create(cb);
            auto callbackB = CallFuncN::create(cb);

            auto seqA = Sequence::create(thereA, callbackA, nullptr);
            auto seqB = Sequence::create(thereB, callbackB, nullptr);

            spriteA->runAction(seqA);
            spriteB->runAction(seqB);
        } else {
            cb = CC_CALLBACK_1(GameLayer::afterSwapBack, this);

            gameboard_->swap(posA, posB);

            auto callbackA = CallFuncN::create(cb);
            auto callbackB = CallFuncN::create(cb);

            auto backA = thereB->clone();
            auto backB = thereA->clone();

            auto seqA = Sequence::create(thereA, backA, callbackA, nullptr);
            auto seqB = Sequence::create(thereB, backB, callbackB, nullptr);

            spriteA->runAction(seqA);
            spriteB->runAction(seqB);

            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("move.aif");
        }

    }

    void GameLayer::cleanup()
    {
        Layer::cleanup();
        if (gameboard_) {
            delete gameboard_;
            gameboard_ = nullptr;
        }
    }

    GameLayer::~GameLayer()
    {
        cleanup();
    }

    void GameLayer::dissapear(Piece * _Piece) {
        _Piece->sprite()->stopAllActions();
        _Piece->sprite()->setLocalZOrder(0);
        auto dissapear = Sequence::create(
                ScaleTo::create(Gameboard::FalldownSpeed, 0.0f),
                RemoveSelf::create(true),
                // TODO:
                ///CallFunc::create(CC_CALLBACK_0()), //delete piece;
                nullptr);
        _Piece->sprite()->runAction(dissapear);
    }

//void GameLayer::menuCloseCallback(Ref* pSender) {
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
//    MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
//    return;
//#endif
//
//    Director::getInstance()->end();
//
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//    exit(0);
//#endif
//}
}
