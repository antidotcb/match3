/*
 * Board.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#include "Board.h"

#include <2d/CCActionInstant.h>
#include <2d/CCActionInterval.h>
#include <2d/CCLayer.h>
#include <2d/CCSprite.h>
#include <base/ccMacros.h>
#include <base/CCPlatformMacros.h>
#include <math/Vec2.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

#include "Piece.h"

USING_NS_CC;

namespace match3 {
    const float Gameboard::FalldownSpeed = .2f;

    const char* Gameboard::BgSpriteTextureName = "background.png";
    const cocos2d::Vec2 Gameboard::DefaultOrigin(120, 120);

    float Gameboard::fillup() {
        float longestAnimation = 0;
        static const float ElasticModifier = 3.0f;
        newPieces_.clear();

        std::vector<std::queue<uint16_t> > empty;
        empty.resize(width_);
        for (uint16_t x = 0; x < width_; x++) {
            for (uint16_t y = 0; y < heigth_; y++) {
                Coord pos(x, y);
                Piece* piece = getPiece(pos);
                if (!piece) {
                    empty[x].push(y);
                } else {
                    if (!empty[x].empty()) {
                        uint16_t emptyY = empty[x].front();
                        empty[x].pop();
                        Coord newPos(x, emptyY);
                        swap(pos, newPos);

                        Vec2 screenPos = cellToScreen(newPos);
                        uint16_t distance = y - emptyY;
                        float animationTime = FalldownSpeed * distance;

                        auto movedown = MoveTo::create(animationTime, screenPos);
                        auto falldown = EaseIn::create(movedown, ElasticModifier);

                        piece->sprite()->runAction(falldown);

                        empty[x].push(y);

                        newPieces_.push_back(piece);

                        longestAnimation = (longestAnimation < animationTime) ? animationTime : longestAnimation ;
                    }
                }
            }

            CCLOG("Row #%d contain %d empty slots", x, empty[x].size());

            uint16_t newlyCreated = 0;
            uint16_t emptySlots = empty[x].size();
            while (!empty[x].empty()) {
                uint16_t y = empty[x].front();
                empty[x].pop();
                Coord pos(x, y);
                Coord ontop = Coord(x, heigth_ + newlyCreated++);

                Piece* piece = factory_->createPiece();

                setPiece(pos, piece);

                layer_->addChild(piece->sprite(), FgSpriteLevel);

                Vec2 targetPos = cellToScreen(pos);
                Vec2 startPos = cellToScreen(ontop);

                float animationTime = FalldownSpeed * emptySlots;

                auto movedown = MoveTo::create(animationTime, targetPos);
                auto falldown = EaseIn::create(movedown, ElasticModifier);

                piece->sprite()->setPosition(startPos);
                piece->sprite()->runAction(falldown);

                newPieces_.push_back(piece);

                longestAnimation = (longestAnimation < animationTime) ? animationTime : longestAnimation ;

            }
        }

        return longestAnimation;
    }

    void Gameboard::remove(Piece* _Piece) {
        if (!_Piece) {
            // TODO:
            return;
        }

        Piece * piece = getPiece(_Piece->position());
        if (piece == _Piece) {
            setPiece(piece->position(), nullptr);
        }
    }

    void Gameboard::checkDirection(bool _Horizontal) {
        std::vector<Piece*> container;

        const uint16_t & c_size = _Horizontal ? width_ : heigth_;
        const uint16_t & r_size = _Horizontal ? heigth_ : width_;
        container.reserve(c_size);

        for (uint16_t i = 0; i < r_size; i++) {
            uint16_t lastType = -1;
            uint16_t typeInRow = 0;

            container.clear();

            for (uint16_t j = 0; j < c_size; j++) {
                const uint16_t & x = _Horizontal ? j : i;
                const uint16_t & y = _Horizontal ? i : j;
                Coord pos(x, y);
                Piece* piece = getPiece(pos);
                if (piece->type() == lastType && j != 0) {
                    typeInRow++;
                    if (typeInRow > 3) {
                        container.push_back(piece);
                    } else if (typeInRow == 3) {
                        for (uint16_t before = 0; before < 3; before++) {
                            const uint16_t & bx = _Horizontal ? x - before : x;
                            const uint16_t & by = _Horizontal ? y : y - before;
                            Coord pos3(bx, by);
                            container.push_back(getPiece(pos3));
                        }
                    }
                } else {
                    if (!container.empty()) {
                        piecesToRemove_.push_front(container);
                        container.clear();
                    }
                    typeInRow = 1;
                    lastType = piece->type();
                }
            }

            if (!container.empty()) {
                piecesToRemove_.push_front(container);
                container.clear();
            }
        }
    }

    bool Gameboard::check() {
        piecesToRemove_.clear();

        static const bool horizontal = true;
        static const bool vertical = false;

        checkDirection(horizontal);
        checkDirection(vertical);

        return (piecesToRemove_.size() > 0);
    }

    void Gameboard::getResultsOfLastCheck(std::list<std::vector<Piece*> > &_Container) {
        _Container.clear();
        _Container.swap(piecesToRemove_);
    }

    void Gameboard::lock() {
        locked_ = true;
    }

    void Gameboard::unlock() {
        locked_ = false;
    }

    bool Gameboard::locked() const {
        return locked_;
    }

    Gameboard::Gameboard(const Size& _Size, IAbstractPieceFactory* _Factory, cocos2d::Layer * _Layer) :
            board_(0), width_(_Size.width), heigth_(_Size.height), factory_(_Factory), layer_(_Layer), locked_(false) {
        CCLOG("Gameboard created");
    }

    bool Gameboard::init() {
        CCLOGINFO("Init started");

        if (board_) {
            CCLOGINFO("Board existed. Remove existing board.");
            cleanup();
        }

        if (!validate()) {
            CCLOGERROR("Validation of init parameters failed.");
            return false;
        }

        board_ = new BoardRow[heigth_];
        if (!board_) {
            CCLOGERROR("Cannot allocate memory for board rows.");
            return false;
        }

        memset(board_, 0, sizeof(BoardRow) * heigth_);
        for (uint16_t y = 0; y < heigth_; y++) {
            board_[y] = new BoardPiece[width_];
            if (!board_[y]) {
                CCLOGERROR("Cannot allocate memory for board pieces.");
                cleanup();
                return false;
            }
            memset(board_[y], 0, sizeof(BoardPiece) * width_);
            for (uint16_t x = 0; x < width_; x++) {
                Coord pos(x, y);
                Piece* piece = factory_->createPiece();
                if (!piece) {
                    CCLOGERROR("Cannot create piece.");
                    cleanup();
                    return false;
                }
                setPiece(pos, piece);

                Vec2 layerPos = cellToScreen(pos);

                piece->sprite()->setPosition(layerPos);
                layer_->addChild(piece->sprite(), FgSpriteLevel);

                Sprite* bgSprite = Sprite::create(BgSpriteTextureName);

                bgSprite->setPosition(layerPos);
                layer_->addChild(bgSprite, BgSpriteLevel);

                sprites_.push_back(bgSprite);
            }
        }

        CCLOG("Gameboard inits finished succesfully.");
        return true;
    }

    Gameboard* Gameboard::create(const cocos2d::Vec2 _Position, const Size& _Size, IAbstractPieceFactory* _Factory, cocos2d::Layer* _Layer) {
        Gameboard* pRet = new Gameboard(_Size, _Factory, _Layer);
        if (pRet && pRet->init()) {
            //TODO: correct
            //pRet->cleanup();
            return pRet;
        } else {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }

    void Gameboard::swap(const Coord& _Pos1, const Coord& _Pos2) {
        Piece* piece1 = getPiece(_Pos1);
        Piece* piece2 = getPiece(_Pos2);

        setPiece(_Pos2, piece1);
        setPiece(_Pos1, piece2);
    }

    Gameboard::~Gameboard()
    {
        cleanup();
    }

    void Gameboard::cleanup() {
        if (board_) {
            for (uint16_t y = 0; y < heigth_; y++) {
                if (board_[y]) {
                    for (uint16_t x = 0; x < width_; x++) {
                        Coord pos(x, y);
                        Piece* piece = getPiece(pos);
                        if (piece) {
                            delete piece;
                            setPiece(pos, 0);
                        }
                    }
                    delete[] board_[y];
                    board_[y] = 0;
                }
            }
            delete[] board_;
            board_ = 0;
        }
    }

    bool Gameboard::validate() {
        if (!factory_) {
            CCLOGERROR("No piece factory given.");
            return false;
        }

        if (!width_ || !heigth_) {
            CCLOGERROR("Wrong size [%d x %d]", width_, heigth_);
            return false;
        }

        if (!layer_) {
            CCLOGERROR("No layer given.");
            return false;
        }
        return true;
    }

    cocos2d::Vec2
    Gameboard::cellToScreen(const Coord & _Coord) {
        cocos2d::Vec2 pos = origin_;
        const uint16_t & x = _Coord.X;
        const uint16_t & y = _Coord.Y;
        pos.x += (x * CellSize) + (x > 1 ? (x - 1) * CellPadding : 0) + CellSize / 2;
        pos.y += (y * CellSize) + (y > 1 ? (y - 1) * CellPadding : 0) + CellSize / 2;
        return pos;
    }

    Coord
    Gameboard::screenToCell(const cocos2d::Vec2 _TouchPos) {
        Coord pos(0, 0);
        pos.X = (_TouchPos.x - origin_.x) / (CellSize + CellPadding);
        pos.Y = (_TouchPos.y - origin_.y) / (CellSize + CellPadding);
        return pos;
    }

    Piece* Gameboard::getPiece(Coord _Coord) {
        if ((_Coord.X >= width_) || (_Coord.Y >= heigth_)) {
            // TODO
            return nullptr;
        }

        return (board_[_Coord.Y][_Coord.X]);
    }

    void Gameboard::setPiece(Coord _Coord, Piece* _Piece) {
        if ((_Coord.X >= width_) || (_Coord.Y >= heigth_)) {
            // TODO
        }
        board_[_Coord.Y][_Coord.X] = _Piece;
        if (_Piece) {
            _Piece->setPosition(_Coord);
        }
    }

} /* namespace match3 */
