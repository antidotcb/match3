/*
 * Board.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#include "Board.h"
#include "Piece.h"

USING_NS_CC;

namespace match3 {

    const char* Gameboard::BgSpriteTextureName = "background.png";
    const cocos2d::Vec2 Gameboard::Origin(100, 100);

    bool Gameboard::check() {
        for (uint16_t y = 0; y < heigth_; y++) {
            for (uint16_t x = 0; x < width_; x++) {
                Coord pos(x, y);
                Piece* piece = pieceAt(pos);


            }
        }
        return false;
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
                board_[y][x] = piece;

                Vec2 layerPos = cellToScreen(pos);

                Sprite *fgSprite = piece->sprite();
                fgSprite->setPosition(layerPos);
                layer_->addChild(piece->sprite(), FgSpriteLevel);

                piece->setPosition(pos);

                Sprite* bgSprite = Sprite::create(BgSpriteTextureName);
                sprites.push_back(bgSprite);
                bgSprite->setPosition(layerPos);

                layer_->addChild(bgSprite, BgSpriteLevel);
            }
        }

        CCLOG("Gameboard inits finished succesfully.");
        return true;
    }

    Gameboard* Gameboard::create(const Size& _Size, IAbstractPieceFactory* _Factory, cocos2d::Layer* _Layer) {
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

    bool Gameboard::swap(const Coord& _Pos1, const Coord& _Pos2) {
        Piece* piece1 = pieceAt(_Pos1);
        Piece* piece2 = pieceAt(_Pos2);
        if (!piece1 || !piece2) {
            return false;
        }
        piece1->setPosition(_Pos2);
        setPiece(_Pos2, piece1);
        piece2->setPosition(_Pos1);
        setPiece(_Pos1, piece2);
        return true;
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
                        Piece* piece = pieceAt(pos);
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
        cocos2d::Vec2 pos = Origin;
        const uint16_t & x = _Coord.X;
        const uint16_t & y = _Coord.Y;
        pos.x += (x * CellSize) + (x > 1 ? (x - 1) * CellPadding : 0) + CellSize / 2;
        pos.y += (y * CellSize) + (y > 1 ? (y - 1) * CellPadding : 0) + CellSize / 2;
        return pos;
    }

    Coord
    Gameboard::screenToCell(const cocos2d::Vec2 _TouchPos) {
        Coord pos(0, 0);
        pos.X = (_TouchPos.x - Origin.x) / (CellSize + CellPadding);
        pos.Y = (_TouchPos.y - Origin.y) / (CellSize + CellPadding);
        return pos;
    }

    Piece* Gameboard::pieceAt(Coord _Coord) {
        if ((_Coord.X >= width_) || (_Coord.Y >= heigth_)) {
            // TODO: Exception situation - out of range
            return nullptr;
        }
        // TODO:
        return (board_[_Coord.Y][_Coord.X]);
    }

    void Gameboard::setPiece(Coord _Coord, Piece* _Piece) {
        if ((_Coord.X >= width_) || (_Coord.Y >= heigth_)) {
            // TODO: Exceptional situation - out of range
        }
        board_[_Coord.Y][_Coord.X] = _Piece;
    }

} /* namespace match3 */
