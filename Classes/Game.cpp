#include <SimpleAudioEngine.h>
#include <cstdint>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "Game.h"
#include "Board.h"
#include "Piece.h"

USING_NS_CC;

namespace match3 {
    const float GameLayer::TotalGameTime = 60;

    const int GameLayer::DefaultBoardSize = 8;
    const int GameLayer::BackgroundLayerLevel = -100;
    const int GameLayer::HighlightActionsTag = 50;

    const float GameLayer::FastSpeed = .25f;
    const float GameLayer::SlowSpeed = .5f;
    const float GameLayer::DissapearSpeed = .1f;

    const float GameLayer::FastSpeedLL = GameLayer::FastSpeed + (GameLayer::FastSpeed / 10.0f);
    const float GameLayer::SlowSpeedLL = GameLayer::SlowSpeed + (GameLayer::SlowSpeed / 10.0f);

    const char* GameLayer::BackgroundTextureName = "bg3.png";

    Scene* GameLayer::wrapIntoScene() {
        auto scene = Scene::create();
        auto layer = GameLayer::create();
        scene->addChild(layer);
        return scene;
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
        addProgressTimer();
        addScores();
        //addUIElements();

        //CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("music.mp3", true);
        return true;
    }

    void GameLayer::addBackground()
    {
        Sprite* sprite = Sprite::create(BackgroundTextureName);
        sprite->setPosition(Vec2(visibleSize_.width / 2 + origin_.x,
                visibleSize_.height / 2 + origin_.y));
        addChild(sprite, BackgroundLayerLevel);

        auto backgroundPS = ParticleRain::createWithTotalParticles(50);
        auto texture = Director::getInstance()->getTextureCache()->addImage("stars.png");
        backgroundPS->setTexture(texture);
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
        gameboard_->setPosition(pos);
        addChild(gameboard_);

        gameboard_->fillup(PiecesManager::getInstance(), false);

        while (gameboard_->check()) {
            std::list<std::vector<Piece*> > removePieces;
            gameboard_->getResultsOfLastCheck(removePieces);
            for (auto vec : removePieces) {
                for (Piece* piece : vec) {
                    gameboard_->remove(piece);
                    piece->sprite()->removeFromParentAndCleanup(true);
                    //delete piece;
                }
            }
            gameboard_->fillup(PiecesManager::getInstance(), false);
        }
    }

    void GameLayer::addInputDispatcher()
    {
        EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();

        listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
        listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);

        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    }

    bool GameLayer::onTouchBegan(cocos2d::Touch *_Touch, cocos2d::Event *_Event) {
        Vec2 touch = _Touch->getLocation();
        Coord coord = gameboard_->wolrd2coord(touch);
        Piece* piece = gameboard_->getPiece(coord);

        CCLOG("Touch: [x=%f, y=%f], Piece: [x=%d, y=%d]",
                touch.x, touch.y, coord.x, coord.y);

        if (gameboard_->locked() || !piece) {
            return true;
        }

        CCLOG("Piece Type: %d", piece->type());

        if (selected_ && selected_->isNextTo(piece)) {
            swapPieces(selected_, piece);
        } else {
            select(piece);
        }

        return true;
    }

    void GameLayer::onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event) {
        Vec2 touch = _Touch->getLocation();
        Coord coord = gameboard_->wolrd2coord(touch);
        Piece* piece = gameboard_->getPiece(coord);

        CCLOG("Touch: [x=%f, y=%f], Piece: [x=%d, y=%d]",
                touch.x, touch.y, coord.x, coord.y);

        if (gameboard_->locked() || !piece) {
            return;
        }

        CCLOGINFO("Piece Type: %d", piece->type());

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

        score_->newSequence(removePieces.size());
        for (auto vec : removePieces) {
            uint32_t size = vec.size();
            CCLOG("Found %d pieces in row", size);

            uint scoreValue = score_->addGroup(vec.size());
            for (Piece* piece : vec) {
                gameboard_->remove(piece);

                Vec2 pos = gameboard_->coord2world(piece->coord());
                addScoreLabel(scoreValue, pos);
                addSomeStars(pos);

                dissapear(piece);
            }

            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("remove.wav");
        }

        float longestAnimationTime = gameboard_->fillup(PiecesManager::getInstance());
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
                    DelayTime::create(waitFalldownToComplete),
                    CallFuncN::create(cb),
                    CallFuncN::create(cb),
                    nullptr);

            runAction(waitToRecheck);
        }
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

    void ScoreToString(uint _Score, std::string& _Str) {
        std::stringstream ss;
        ss << _Score;
        ss >> _Str;
    }

    void GameLayer::onProgressTimer() {
        time += delayTime;
        float percent = 100 - (time / TotalGameTime) * 100;
        timer_->setPercentage(percent);
    }

    void GameLayer::onTimeExpires() {
        CCLOG("Time is out");
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

    void GameLayer::addSomeStars(const Vec2 & _Position) {
        auto ps = ParticleExplosion::createWithTotalParticles(5);
        auto texture = Director::getInstance()->getTextureCache()->addImage("stars2.png");
        ps->setPosition(_Position);
        ps->setTexture(texture);
        ps->setOpacity(127);
        addChild(ps, ScoreLabelsLayerLevel - 1);

        auto removeAfterDelay = Sequence::create(
                FadeOut::create(SlowSpeed),
                RemoveSelf::create(true),
                nullptr);
        ps->runAction(removeAfterDelay);
    }

    void GameLayer::addScoreLabel(uint _Score, const Vec2 & _Position) {
        std::string scoreText;
        ScoreToString(_Score, scoreText);
        CCLOG("Score %d == %s", _Score, scoreText.c_str());

        auto label = Label::createWithBMFont("fonts/overlay.fnt", scoreText, TextHAlignment::CENTER);

        label->setColor(Color3B::YELLOW);
        label->enableShadow(Color4B::BLACK, Size(2.f, 2.f), 5);
        //label->enableOutline(Color4B::RED, 1);

        addChild(label, ScoreLabelsLayerLevel);

        label->setPosition(_Position);

        auto dissapear = Spawn::create(
                MoveBy::create(SlowSpeed, Vec2(0, 20)),
                FadeTo::create(SlowSpeed, 64),
                ScaleBy::create(SlowSpeed, 0.3f),
                nullptr);

        auto seq = Sequence::create(
                DelayTime::create(DissapearSpeed),
                dissapear,
                RemoveSelf::create(true),
                nullptr);

        label->runAction(seq);
    }

    void GameLayer::swapPieces(Piece* _First, Piece* _Second) {
        std::function<void(Node*)> cb;

        gameboard_->lock();
        deselect();

        Sprite* spriteA = _First->sprite();
        Sprite* spriteB = _Second->sprite();

        Coord posA = _First->coord();
        Coord posB = _Second->coord();

        auto thereA = MoveTo::create(FastSpeed, spriteB->getPosition());
        auto thereB = MoveTo::create(FastSpeed, spriteA->getPosition());

        gameboard_->swap(posA, posB);

        if (gameboard_->check()) {
            score_->newTurn();

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
