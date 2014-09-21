/*
 * Gameboardlayer.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: antidotcb
 */

#include "GameboardLayer.h"

#include <cocos2d.h>
#include <SimpleAudioEngine.h>
#include <cstdint>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "Board.h"
#include "Piece.h"
#include "Score.h"

namespace match3 {
    USING_NS_CC;

    const int GameBoardLayer::HighlightActionsTag = 50;

    const float GameBoardLayer::FastSpeed = .25f;
    const float GameBoardLayer::SlowSpeed = .5f;
    const float GameBoardLayer::DissapearSpeed = .1f;

    const float GameBoardLayer::FastSpeedLL = GameBoardLayer::FastSpeed + (GameBoardLayer::FastSpeed / 10.0f);
    const float GameBoardLayer::SlowSpeedLL = GameBoardLayer::SlowSpeed + (GameBoardLayer::SlowSpeed / 10.0f);

    bool GameBoardLayer::onTouchBegan(cocos2d::Touch *_Touch, cocos2d::Event *_Event) {
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

    void GameBoardLayer::onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event) {
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

    void GameBoardLayer::select(Piece* _Piece) {
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

    void GameBoardLayer::deselect() {
        if (!selected_) {
            return;
        }

        selected_->sprite()->stopActionByTag(HighlightActionsTag);
        selected_->sprite()->runAction(FadeTo::create(.0f, 255));
        selected_ = nullptr;
    }

    void GameBoardLayer::afterSwap(Node* _Node) {
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

            uint32_t scoreValue = score_->addGroup(vec.size());
            for (Piece* piece : vec) {
                gameboard_->remove(piece);

                Vec2 pos = gameboard_->coord2world(piece->getCoords());
                addScoreLabel(scoreValue, piece->sprite()->getPosition());
                addSomeEffects(piece->sprite()->getPosition());

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
            cb = CC_CALLBACK_1(GameBoardLayer::afterSwap, this);

            // Hack - need to call it twice to make it do the job
            auto waitToRecheck = Sequence::create(
                    DelayTime::create(waitFalldownToComplete),
                    CallFuncN::create(cb),
                    CallFuncN::create(cb),
                    nullptr);

            runAction(waitToRecheck);
        }
    }

    void GameBoardLayer::afterSwapBack(Node* _Node) {
        if (firstArrived_ == nullptr) {
            CCLOG("swapBack: First arrived, proceed further.");
            firstArrived_ = _Node;
            return;
        }

        firstArrived_ = nullptr;

        gameboard_->unlock();
        CCLOG("swapBack: Second arrived, finally can unlock board.");
    }

    void ScoreToString(uint32_t _Score, std::string& _Str) {
        std::stringstream ss;
        ss << _Score;
        ss >> _Str;
    }

    void GameBoardLayer::addSomeEffects(const Vec2 & _Position) {
        auto ps = ParticleExplosion::createWithTotalParticles(5);
        auto texture = Director::getInstance()->getTextureCache()->addImage("stars.png");
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

    void GameBoardLayer::addScoreLabel(uint32_t _Score, const Vec2 & _Position) {
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

    void GameBoardLayer::swapPieces(Piece* _First, Piece* _Second) {
        std::function<void(Node*)> cb;

        gameboard_->lock();
        deselect();

        Sprite* spriteA = _First->sprite();
        Sprite* spriteB = _Second->sprite();

        Coord posA = _First->getCoords();
        Coord posB = _Second->getCoords();

        auto thereA = MoveTo::create(FastSpeed, spriteB->getPosition());
        auto thereB = MoveTo::create(FastSpeed, spriteA->getPosition());

        gameboard_->swap(posA, posB);

        if (gameboard_->check()) {
            score_->newTurn();

            cb = CC_CALLBACK_1(GameBoardLayer::afterSwap, this);

            auto callbackA = CallFuncN::create(cb);
            auto callbackB = CallFuncN::create(cb);

            auto seqA = Sequence::create(thereA, callbackA, nullptr);
            auto seqB = Sequence::create(thereB, callbackB, nullptr);

            spriteA->runAction(seqA);
            spriteB->runAction(seqB);
        } else {
            cb = CC_CALLBACK_1(GameBoardLayer::afterSwapBack, this);

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

    void GameBoardLayer::dissapear(Piece * _Piece) {
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

    GameBoardLayer* GameBoardLayer::create(Gameboard* _Gameboard, Score* _Score)
            {
        GameBoardLayer *pRet = new GameBoardLayer(_Gameboard, _Score);
        if (pRet && pRet->init())
                {
            pRet->autorelease();
            return pRet;
        }
        else
        {
            delete pRet;
            pRet = NULL;
            return NULL;
        }
    }

    void GameBoardLayer::cleanup()
    {
    }

    bool GameBoardLayer::init()
    {
        if (!gameboard_) {
            return false;
        }
        if (!score_) {
            return false;
        }

        addChild(gameboard_);

        //gameboard_->clean();
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

        return true;
    }

    GameBoardLayer::GameBoardLayer(Gameboard* _Gameboard, Score* _Score)
    :
            gameboard_(_Gameboard), score_(_Score)
    {
    }

} /* namespace match3 */
