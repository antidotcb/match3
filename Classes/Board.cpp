/*
 * Board.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#include "Board.h"

#include <2d/CCActionEase.h>
#include <2d/CCActionInterval.h>
#include <2d/CCSprite.h>
#include <base/CCPlatformMacros.h>
#include <cstring>
#include <queue>


USING_NS_CC;

namespace match3 {
    const float Gameboard::FalldownSpeed = .2f;

    const char* Gameboard::BgSpriteTextureName = "background.png";
    const float Gameboard::CellSize = 40;
    const float Gameboard::HalfCellSize = CellSize / 2.f;

    Vec2 Gameboard::center() const
    {
        // FIX: in case of dynamic size it won't work, but now it's ok
        static Vec2 center(width_ * CellSize / 2.f, height_ * CellSize / 2.f);
        return center;
    }

    float Gameboard::fillup(IAbstractPieceFactory* _PieceFactory, bool animate) {
        // FIX: move animation out of there as Gameboard isn't responsible for this
        float longestAnimation = 0;
        static const float ElasticModifier = 3.0f;
        newPieces_.clear();

        std::vector<std::queue<uint16_t> > empty;
        empty.resize(width_);
        for (uint16_t x = 0; x < width_; x++) {
            for (uint16_t y = 0; y < height_; y++) {
                Coord coord(x, y);
                Piece* piece = getPiece(coord);
                if (!piece) {
                    empty[x].push(y);
                } else {
                    if (!empty[x].empty()) {
                        uint16_t emptyY = empty[x].front();

                        Coord newPos(x, emptyY);
                        swap(coord, newPos);

                        Vec2 screenPos = coord2local(newPos);
                        uint16_t distance = y - emptyY;
                        float animationTime = FalldownSpeed * distance;

                        auto movedown = MoveTo::create(animationTime, screenPos);
                        auto falldown = EaseIn::create(movedown, ElasticModifier);

                        if (animate) {
                            piece->sprite()->runAction(falldown);
                        } else {
                            piece->sprite()->setPosition(screenPos);
                        }

                        empty[x].push(y);

                        newPieces_.push_back(piece);
                        empty[x].pop();
                    }
                }
            }

            CCLOG("Row #%d contain %d empty slots", x, empty[x].size());

            uint16_t newlyCreated = 0;
            uint16_t emptySlots = empty[x].size();
            while (!empty[x].empty()) {
                uint16_t y = empty[x].front();
                empty[x].pop();
                Coord coord(x, y);
                Coord ontop = Coord(x, height_ + newlyCreated++);

                Piece* piece = _PieceFactory->createPiece(coord);

                setPiece(coord, piece);

                addChild(piece->sprite(), FgSpriteLevel);

                Vec2 targetPos = coord2local(coord);
                Vec2 startPos = coord2local(ontop);

                float animationTime = FalldownSpeed * emptySlots;
                longestAnimation = (longestAnimation < animationTime) ? animationTime : longestAnimation;

                auto movedown = MoveTo::create(animationTime, targetPos);
                auto falldown = EaseIn::create(movedown, ElasticModifier);

                if (animate) {
                    piece->sprite()->setPosition(startPos);
                    piece->sprite()->runAction(falldown);
                } else {
                    piece->sprite()->setPosition(targetPos);
                }

                newPieces_.push_back(piece);
            }
        }

        return longestAnimation;
    }

    void Gameboard::remove(Piece* _Piece) {
        Piece * piece = getPiece(_Piece->getCoords());
        if (piece == _Piece) {
            setPiece(piece->getCoords(), nullptr);
        }
    }

    void Gameboard::checkDirection(bool _Horizontal) {
        std::vector<Piece*> container;

        const uint16_t & c_size = _Horizontal ? width_ : height_;
        const uint16_t & r_size = _Horizontal ? height_ : width_;
        container.reserve(c_size);

        for (uint16_t i = 0; i < r_size; i++) {
            uint16_t lastType = -1;
            uint16_t typeInRow = 0;

            container.clear();

            for (uint16_t j = 0; j < c_size; j++) {
                const uint16_t & x = _Horizontal ? j : i;
                const uint16_t & y = _Horizontal ? i : j;
                Coord coord(x, y);
                Piece* piece = getPiece(coord);
                if (piece->type() == lastType && j != 0) {
                    typeInRow++;
                    if (typeInRow > 3) {
                        container.push_back(piece);
                    } else if (typeInRow == 3) {
                        for (uint16_t before = 0; before < 3; before++) {
                            coord.x = _Horizontal ? x - before : x;
                            coord.y = _Horizontal ? y : y - before;
                            container.push_back(getPiece(coord));
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

    Gameboard::Gameboard(const Size& _Size) :
            board_(0), width_(_Size.width), height_(_Size.height), locked_(false) {
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

        board_ = new BoardRow[height_];
        if (!board_) {
            CCLOGERROR("Cannot allocate memory for board rows.");
            return false;
        }

        memset(board_, 0, sizeof(BoardRow) * height_);
        for (uint16_t y = 0; y < height_; y++) {
            board_[y] = new BoardPiece[width_];
            if (!board_[y]) {
                CCLOGERROR("Cannot allocate memory for board pieces.");
                cleanup();
                return false;
            }
            memset(board_[y], 0, sizeof(BoardPiece) * width_);
            for (uint16_t x = 0; x < width_; x++) {
                Coord coord(x, y);
                Vec2 pos = coord2local(coord);

                setPiece(coord, nullptr);

                Sprite* bgSprite = Sprite::create(BgSpriteTextureName);
                bgSprite->setPosition(pos);
                addChild(bgSprite, BgSpriteLevel);
            }
        }

        CCLOG("Gameboard inits finished succesfully.");
        return true;
    }

    Gameboard* Gameboard::create(const Size& _Size) {
        Gameboard* pRet = new Gameboard(_Size);
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
            for (uint16_t y = 0; y < height_; y++) {
                if (board_[y]) {
                    for (uint16_t x = 0; x < width_; x++) {
                        Coord coord(x, y);
                        Piece* piece = getPiece(coord);
                        if (piece) {
                            delete piece;
                            setPiece(coord, nullptr);
                        }
                    }
                    delete[] board_[y];
                    board_[y] = nullptr;
                }
            }
            delete[] board_;
            board_ = nullptr;
        }
    }

    bool Gameboard::validate() {
        if (!width_ || !height_) {
            CCLOGERROR("Wrong size [%d x %d]", width_, height_);
            return false;
        }

        return true;
    }

    Vec2 Gameboard::coord2local(const Coord & _Coord) {
        Vec2 pos;
        const uint16_t & x = _Coord.x;
        const uint16_t & y = _Coord.y;
        pos.x = (x * CellSize) + HalfCellSize - center().x;
        pos.y = (y * CellSize) + HalfCellSize - center().y;
        return pos;
    }

    Vec2 Gameboard::coord2world(const Coord & _Coord) {
        Vec2 local = coord2local(_Coord);
        return convertToWorldSpace(local);
    }

    Coord Gameboard::wolrd2coord(const cocos2d::Vec2& _TouchPos) {
        Vec2 local = convertToNodeSpace(_TouchPos);
        return local2coord(local);
    }

    Coord Gameboard::local2coord(const cocos2d::Vec2 & _Pos) {
        Coord coord(0, 0);
        coord.x = (_Pos.x + center().x) / CellSize;
        coord.y = (_Pos.y + center().y) / CellSize;
        return coord;
    }

    Piece* Gameboard::getPiece(Coord _Coord) {
        if ((_Coord.x >= width_) || (_Coord.y >= height_)) {
            // TODO
            return nullptr;
        }

        return (board_[_Coord.y][_Coord.x]);
    }

    void Gameboard::setPiece(Coord _Coord, Piece* _Piece) {
        if ((_Coord.x >= width_) || (_Coord.y >= height_)) {
            // TODO
        }
        board_[_Coord.y][_Coord.x] = _Piece;
        if (_Piece) {
            _Piece->setCoords(_Coord);
        }
    }

} /* namespace match3 */
