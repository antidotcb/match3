/*
 * Gameboardlayer.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: antidotcb
 */

#include "Gameboard.h"

#include <2d/CCAction.h>
#include <2d/CCActionEase.h>
#include <2d/CCActionInstant.h>
#include <2d/CCActionInterval.h>
#include <2d/CCLabel.h>
#include <2d/CCParticleExamples.h>
#include <2d/CCSprite.h>
#include <base/ccMacros.h>
#include <base/ccTypes.h>
#include <base/CCDirector.h>
#include <base/CCPlatformMacros.h>
#include <base/CCTouch.h>
#include <math/CCGeometry.h>
#include <renderer/CCTextureCache.h>
#include <SimpleAudioEngine.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "Board.h"
#include "Piece.h"
#include "Score.h"

namespace match3 {
    USING_NS_CC;

    const char* Gameboard::BgSpriteTextureName = "background.png";

    const float Gameboard::CellSize = 40;
    const float Gameboard::HalfCellSize = Gameboard::CellSize / 2.f;
    const float Gameboard::ElasticModifier = 3.0f;

    const float Gameboard::FalldownSpeed = .1f;
    const float Gameboard::DissapearSpeed = .1f;
    const float Gameboard::FastSpeed = .25f;
    const float Gameboard::SlowSpeed = .5f;

    const float Gameboard::FastSpeedLL = Gameboard::FastSpeed + (Gameboard::FastSpeed / 10.0f);
    const float Gameboard::SlowSpeedLL = Gameboard::SlowSpeed + (Gameboard::SlowSpeed / 10.0f);

    bool Gameboard::onTouchBegan(cocos2d::Touch *_Touch, cocos2d::Event *_Event) {
        Vec2 touch = _Touch->getLocation();
        Coord coord = wolrd2coord(touch);
        IPiece* piece = board_->pieceAt(coord);

        CCLOGINFO("Touch: [x=%f, y=%f], Piece: [x=%d, y=%d]",
                touch.x, touch.y, coord.x, coord.y);

        if (locked() || !piece) {
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

    void Gameboard::onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event) {
        Vec2 touch = _Touch->getLocation();
        Coord coord = wolrd2coord(touch);
        IPiece* piece = board_->pieceAt(coord);

        CCLOGINFO("Touch: [x=%f, y=%f], Piece: [x=%d, y=%d]",
                touch.x, touch.y, coord.x, coord.y);

        if (locked() || !piece) {
            return;
        }

        CCLOGINFO("Piece Type: %d", piece->type());

        if (selected_ && selected_->isNextTo(piece)) {
            swapPieces(selected_, piece);
        }
    }

    void Gameboard::select(IPiece* _Piece) {
        if (selected_) {
            deselect();
        }

        selected_ = _Piece;

        auto selectSequence = Sequence::create(
                FadeTo::create(SlowSpeed, 192),
                FadeTo::create(SlowSpeed, 255),
                nullptr);

        auto pulsation = RepeatForever::create(selectSequence);
        pulsation->setTag(HighlightTag);

        selected_->sprite()->runAction(pulsation);
    }

    void Gameboard::deselect() {
        if (!selected_) {
            return;
        }

        selected_->sprite()->stopActionByTag(HighlightTag);
        selected_->sprite()->runAction(FadeTo::create(.0f, 255));
        selected_ = nullptr;
    }

    void Gameboard::afterSwap(Node* _Node) {
        if (firstArrived_ == nullptr) {
            CCLOG("checkSwapValid: First arrived, proceed further.");
            firstArrived_ = _Node;
            return;
        }

        firstArrived_ = nullptr;

        CCLOG("checkSwapValid: Second arrived, finally can check.");

        std::set<IPiece*> removal;
        score_->newSequence(checkResults_.size());
        for (auto vec : checkResults_) {
            uint32_t size = vec.size();
            CCLOG("Found %d pieces in row", size);

            uint32_t scoreValue = score_->addGroup(vec.size());
            for (IPiece* piece : vec) {

                board_->remove(piece->getCoords());

                Vec2 pos = coord2world(piece->getCoords());

                Sprite* sprite = piece->sprite();
                addScoreLabel(scoreValue, sprite->getPosition());
                addSomeEffects(sprite->getPosition());

                dissapear(piece);
            }
            removal.insert(vec.begin(), vec.end());

            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("remove.wav");
        }

        for (auto it = removal.begin(); it != removal.end(); ++it) {
            delete *it;
        }

        float longestAnimationTime = refill() * FalldownSpeed;
        float waitFalldownToComplete = longestAnimationTime + FastSpeed;

        if (!check()) {
            auto waitToUnlock = Sequence::create(
                    DelayTime::create(waitFalldownToComplete),
                    CallFunc::create(CC_CALLBACK_0(Gameboard::unlock, this)),
                    nullptr);
            runAction(waitToUnlock);
        } else {
            std::function<void(Node*)> cb;
            cb = CC_CALLBACK_1(Gameboard::afterSwap, this);

            // Hack - need to call it twice to make it do the job
            auto waitToRecheck = Sequence::create(
                    DelayTime::create(waitFalldownToComplete),
                    CallFuncN::create(cb),
                    CallFuncN::create(cb),
                    nullptr);

            runAction(waitToRecheck);
        }
    }

    void Gameboard::afterSwapBack(Node* _Node) {
        if (firstArrived_ == nullptr) {
            CCLOG("swapBack: First arrived, proceed further.");
            firstArrived_ = _Node;
            return;
        }

        firstArrived_ = nullptr;

        unlock();
        CCLOG("swapBack: Second arrived, finally can unlock board.");
    }

    void ScoreToString(uint32_t _Score, std::string& _Str) {
        std::stringstream ss;
        ss << _Score;
        ss >> _Str;
    }

    void Gameboard::addSomeEffects(const Vec2 & _Position) {
        auto ps = ParticleExplosion::createWithTotalParticles(5);
        auto texture = Director::getInstance()->getTextureCache()->addImage("stars.png");
        ps->setPosition(_Position);
        ps->setTexture(texture);
        ps->setOpacity(127);
        addChild(ps, TopmostLevel - 1);

        auto removeAfterDelay = Sequence::create(
                FadeOut::create(SlowSpeed),
                RemoveSelf::create(true),
                nullptr);
        ps->runAction(removeAfterDelay);
    }

    void Gameboard::falldownPiece(const Coord& _From, const Coord& _To) {
        CCLOG("Falldown in column %d:%d from %d to %d", _From.x, _To.x, _From.y, _To.y);
        IPiece* piece = board_->pieceAt(_To);
        if (!piece) {
            CCLOGERROR("No peace at coord");
        }

        Sprite *sprite = piece->sprite();
        Vec2 targetPos = coord2local(_To);
        Vec2 startPos = coord2local(_From);

        uint16_t distance = abs(_From.y - _To.y);

        float animationTime = FalldownSpeed * distance;

        auto movedown = MoveTo::create(animationTime, targetPos);
        auto falldown = EaseIn::create(movedown, ElasticModifier);

        if (animate_) {
            sprite->setPosition(startPos);
            sprite->runAction(falldown);
        } else {
            sprite->setPosition(targetPos);
        }
    }

    uint16_t Gameboard::falldownColumn(uint16_t _Column) {
        const uint16_t &height = board_->height();
        std::queue<uint16_t> emptyCells;
        for (uint16_t y = 0; y < height; y++) {
            const uint16_t & x = _Column;
            Coord coord(x, y);

            IPiece* piece = board_->pieceAt(coord);

            if (!piece) {
                emptyCells.push(y);
            } else {
                if (!emptyCells.empty()) {
                    Coord target(x, emptyCells.front());

                    board_->swap(coord, target);
                    falldownPiece(coord, target);

                    emptyCells.push(y);
                    emptyCells.pop();
                }
            }
        }

        if (emptyCells.empty()) {
            return height;
        } else {
            return emptyCells.front();
        }
    }

    void Gameboard::fillupColumn(const uint16_t _Column, const uint16_t firstEmptyCell) {
        const uint16_t &height = board_->height();
        uint16_t missingPieces = height - firstEmptyCell;
        uint16_t newlyCreated = 0;
        while (missingPieces) {
            uint16_t y = height - missingPieces--;
            Coord coord(_Column, y);
            IPiece* piece = PiecesManager::getInstance()->createPiece(coord);
            if (piece) {
                board_->add(coord, piece);
                Coord ontop = Coord(_Column, height + newlyCreated++);
                falldownPiece(ontop, coord);
                addChild(piece->sprite());
            } else {
                CCLOGERROR("Can't create piece");
            }
        }
    }

    bool Gameboard::check() {
        checkResults_.erase(checkResults_.begin(), checkResults_.end());
        checkDirection(HORIZONTAL);
        checkDirection(VERTICAL);
        return !checkResults_.empty();
    }

    uint16_t Gameboard::refill() {
        uint16_t maxEmptyCells = 0;
        for (uint16_t column = 0; column < board_->width(); column++) {
            uint16_t firstEmptyCell = falldownColumn(column);
            uint16_t columnEmptyCells = board_->height() - firstEmptyCell;
            if (columnEmptyCells > 0) {
                fillupColumn(column, firstEmptyCell);
                if (columnEmptyCells > maxEmptyCells) {
                    maxEmptyCells = columnEmptyCells;
                }
            }
        }
        return maxEmptyCells;

        // FIX: move animation out of there as Gameboard isn't responsible for this
//        float longestAnimation = 0;
//        static const float ElasticModifier = 3.0f;
//
//        std::vector<std::queue<uint16_t> > empty;
//        empty.resize(board_->width());
//        for (uint16_t x = 0; x < board_->width(); x++) {
//            for (uint16_t y = 0; y < board_->height(); y++) {
//                Coord coord(x, y);
//                IPiece* piece = board_->pieceAt(coord);
//                if (!piece) {
//                    empty[x].push(y);
//                } else {
//                    if (!empty[x].empty()) {
//                        uint16_t emptyY = empty[x].front();
//
//                        Coord newPos(x, emptyY);
//                        board_->swap(coord, newPos);
//
//                        Vec2 screenPos = coord2local(newPos);
//                        uint16_t distance = y - emptyY;
//                        float animationTime = FalldownSpeed * distance;
//
//                        auto movedown = MoveTo::create(animationTime, screenPos);
//                        auto falldown = EaseIn::create(movedown, ElasticModifier);
//
//                        Sprite* sprite = piece->sprite();
//                        if (animate) {
//                            sprite->runAction(falldown);
//                        } else {
//                            sprite->setPosition(screenPos);
//                        }
//
//                        empty[x].push(y);
//                        empty[x].pop();
//                    }
//                }
//            }
//
//            CCLOG("Row # %d contain %u empty slots", x, (uint32_t)empty[x].size());
//
//            uint16_t newlyCreated = 0;
//            uint16_t emptySlots = empty[x].size();
//            while (!empty[x].empty()) {
//                uint16_t y = empty[x].front();
//                empty[x].pop();
//                Coord coord(x, y);
//                Coord ontop = Coord(x, board_->height() + newlyCreated++);
//
//                IPiece* piece = _PieceFactory->createPiece(coord);
//
//                board_->add(coord, piece);
//
//                addChild(piece->sprite(), ForegroundLevel);
//
//                Vec2 targetPos = coord2local(coord);
//                Vec2 startPos = coord2local(ontop);
//
//                float animationTime = FalldownSpeed * emptySlots;
//                longestAnimation = (longestAnimation < animationTime) ? animationTime : longestAnimation;
//
//                auto movedown = MoveTo::create(animationTime, targetPos);
//                auto falldown = EaseIn::create(movedown, ElasticModifier);
//
//                if (animate) {
//                    piece->sprite()->setPosition(startPos);
//                    piece->sprite()->runAction(falldown);
//                } else {
//                    piece->sprite()->setPosition(targetPos);
//                }
//            }
//        }
//
//        return longestAnimation;
    }

    void Gameboard::addBackground()
    {
        for (uint16_t y = 0; y < board_->height(); y++) {
            for (uint16_t x = 0; x < board_->width(); x++) {
                Coord coord(x, y);
                Vec2 pos = coord2local(coord);
                Sprite* bgSprite = Sprite::create(BgSpriteTextureName);
                bgSprite->setPosition(pos);
                addChild(bgSprite, BackgroundLevel);
            }
        }
    }

    void Gameboard::addScoreLabel(uint32_t _Score, const Vec2 & _Position) {
        std::string scoreText;
        ScoreToString(_Score, scoreText);
        CCLOG("Score %d == %s", _Score, scoreText.c_str());

        auto label = Label::createWithBMFont("fonts/overlay.fnt", scoreText, TextHAlignment::CENTER);

        label->setColor(Color3B::YELLOW);
        label->enableShadow(Color4B::BLACK, Size(2.f, 2.f), 5);

        addChild(label, TopmostLevel);

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

    void Gameboard::checkDirection(Direction _Direction) {
        std::vector<IPiece*> container;

        const uint16_t & width = board_->width();
        const uint16_t & height = board_->height();

        const uint16_t & c_size = _Direction == HORIZONTAL ? width : height;
        const uint16_t & r_size = _Direction == HORIZONTAL ? height : width;
        container.reserve(c_size);

        for (uint16_t i = 0; i < r_size; i++) {
            uint16_t lastType = -1;
            uint16_t typeInRow = 0;

            container.clear();

            for (uint16_t j = 0; j < c_size; j++) {
                const uint16_t & x = _Direction == HORIZONTAL ? j : i;
                const uint16_t & y = _Direction == HORIZONTAL ? i : j;
                Coord coord(x, y);
                IPiece* piece = board_->pieceAt(coord);
                if (!piece) {
                    CCLOGERROR("Somehow not filled totally before check. Empty cell: %d, %d", x, y);
                    lastType = -1;
                    typeInRow = 0;
                    continue;
                }
                if (piece->type() == lastType && j != 0) {
                    typeInRow++;
                    if (typeInRow > 3) {
                        container.push_back(piece);
                    } else if (typeInRow == 3) {
                        for (uint16_t before = 0; before < 3; before++) {
                            coord.x = _Direction == HORIZONTAL ? x - before : x;
                            coord.y = _Direction == HORIZONTAL ? y : y - before;
                            container.push_back(board_->pieceAt(coord));
                        }
                    }
                } else {
                    if (!container.empty()) {
                        checkResults_.push_back(container);
                        container.clear();
                    }
                    typeInRow = 1;
                    lastType = piece->type();
                }
            }

            if (!container.empty()) {
                checkResults_.push_back(container);
                container.clear();
            }
        }
    }

    void Gameboard::swapPieces(IPiece* _First, IPiece* _Second) {
        std::function<void(Node*)> cb;

        active_ = true;
        lock();
        deselect();

        Sprite* spriteA = _First->sprite();
        Sprite* spriteB = _Second->sprite();

        Coord posA = _First->getCoords();
        Coord posB = _Second->getCoords();

        auto thereA = MoveTo::create(FastSpeed, spriteB->getPosition());
        auto thereB = MoveTo::create(FastSpeed, spriteA->getPosition());

        board_->swap(posA, posB);

        if (check()) {
            score_->newTurn();

            cb = CC_CALLBACK_1(Gameboard::afterSwap, this);

            auto callbackA = CallFuncN::create(cb);
            auto callbackB = CallFuncN::create(cb);

            auto seqA = Sequence::create(thereA, callbackA, nullptr);
            auto seqB = Sequence::create(thereB, callbackB, nullptr);

            spriteA->runAction(seqA);
            spriteB->runAction(seqB);
        } else {
            cb = CC_CALLBACK_1(Gameboard::afterSwapBack, this);

            board_->swap(posA, posB);

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

    void Gameboard::dissapear(IPiece * _Piece) {
        _Piece->sprite()->stopAllActions();
        _Piece->sprite()->setLocalZOrder(0);
        auto dissapear = Sequence::create(
                ScaleTo::create(FalldownSpeed, 0.0f),
                RemoveSelf::create(true),
                nullptr);
        _Piece->sprite()->runAction(dissapear);
    }

    Gameboard* Gameboard::create(Score* _Score) {
        Gameboard *pRet = new Gameboard(_Score);
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = NULL;
            return NULL;
        }
    }

    void Gameboard::cleanup() {
        delete board_;
    }

    bool Gameboard::init() {
        if (!score_) {
            return false;
        }

        board_ = Board::create(*PiecesManager::getInstance());

        addBackground();

        refill();
        while (check()) {
            std::set<IPiece*> removal;
            uint16_t count = 0;
            for (auto vec : checkResults_) {
                std::stringstream ss;
                for (IPiece* piece : vec) {
                    board_->remove(piece->getCoords());

                    Sprite* sprite = piece->sprite();
                    sprite->removeFromParentAndCleanup(true);
                    count++;
                }
                removal.insert(vec.begin(), vec.end());
            }

            for (auto it = removal.begin(); it != removal.end(); ++it) {
                delete *it;
            }

            refill();
        }

        animate_ = true;
        return true;
    }

    Gameboard::Gameboard(Score* _Score) :
            score_(_Score) {
        CCLOG("%s", __func__);
    }

    cocos2d::Vec2 Gameboard::center() const {
        static Vec2 center(board_->width() * HalfCellSize,
                board_->height() * HalfCellSize);
        return center;
    }

    Vec2 Gameboard::coord2local(const Coord & _Coord) const {
        Vec2 pos;
        const uint16_t & x = _Coord.x;
        const uint16_t & y = _Coord.y;
        pos.x = (x * CellSize) + HalfCellSize - center().x;
        pos.y = (y * CellSize) + HalfCellSize - center().y;
        return pos;
    }

    Vec2 Gameboard::coord2world(const Coord & _Coord) const {
        Vec2 local = coord2local(_Coord);
        return convertToWorldSpace(local);
    }

    Coord Gameboard::wolrd2coord(const cocos2d::Vec2& _TouchPos) const {
        Vec2 local = convertToNodeSpace(_TouchPos);
        return local2coord(local);
    }

    Coord Gameboard::local2coord(const cocos2d::Vec2 & _Pos) const {
        Coord coord(0, 0);
        coord.x = (_Pos.x + center().x) / CellSize;
        coord.y = (_Pos.y + center().y) / CellSize;
        return coord;
    }

    void Gameboard::lock() {
        locked_ = true;
    }

    void Gameboard::unlock() {
        locked_ = false;
        active_ = false;
    }

    bool Gameboard::locked() const {
        return locked_;
    }

} /* namespace match3 */
