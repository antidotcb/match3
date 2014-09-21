/*
 * GameboardContainer.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: antidotcb
 */

#include "GameboardContainer.h"

#include <cocos2d.h>
#include <cstring>
#include <queue>
#include <set>
#include <vector>

#include "Piece.h"

namespace match3 {
    USING_NS_CC;

    BoardSize::BoardSize(uint16_t _W, uint16_t _H) :
            width(_W), height(_H) {
    }

    Board::Board(const BoardSize& _Size, const IAbstractPieceFactory& _Factory) :
            generator_(_Factory), width_(_Size.width), height_(_Size.height) {
        CCLOG("Gameboard created");
    }

    bool Board::init() {
        CCLOGINFO("Init started");

        if (board_) {
            CCLOGINFO("Board existed. Remove existing board.");
            clean();
        }

        if (!width_ || !height_) {
            CCLOGERROR("Wrong size [%d x %d]", width_, height_);
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
                clean();
                return false;
            }
            memset(board_[y], 0, sizeof(BoardPiece) * width_);
            for (uint16_t x = 0; x < width_; x++) {
                Coord coord(x, y);
                if (!initCell(coord)) {
                    clean();
                    return false;
                }
            }
        }

        CCLOG("Gameboard inits finished succesfully.");
        return true;
    }

    bool Board::initCell(const Coord& _Coord) {
        setPiece(_Coord, nullptr);
        return true;
    }

    void Board::clean() {
        if (board_) {
            for (uint16_t y = 0; y < height_; y++) {
                if (board_[y]) {
                    for (uint16_t x = 0; x < width_; x++) {
                        Coord coord(x, y);
                        IPiece* piece = pieceAt(coord);
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
        //Node::cleanup();
    }

    Board::~Board() {
        clean();
    }

    uint16_t Board::height() const {
        return height_;
    }

    uint16_t Board::width() const {
        return width_;
    }

    IPiece* Board::pieceAt(const Coord& _Coord) const {
        if ((_Coord.x < width_) && (_Coord.y < height_)) {
            return (board_[_Coord.y][_Coord.x]);
        }
        return nullptr;
    }

    void Board::remove(const Coord & _Coord) {
        setPiece(_Coord, nullptr);
    }

    void Board::swap(const Coord& _Pos1, const Coord& _Pos2) {
        IPiece* piece1 = pieceAt(_Pos1);
        IPiece* piece2 = pieceAt(_Pos2);

        setPiece(_Pos2, piece1);
        setPiece(_Pos1, piece2);
    }

    void Board::setPiece(const Coord& _Coord, IPiece* _Piece) {
        if ((_Coord.x < width_) && (_Coord.y < height_)) {
            CCLOGINFO("%s: x=%d, y=%d", __func__, _Coord.x, _Coord.y);
            IPiece* old = pieceAt(_Coord);
            if (old) {
                remove(_Coord);
            }
            board_[_Coord.y][_Coord.x] = _Piece;
            if (_Piece) {
                _Piece->setCoords(_Coord);
            }

        }
    }

    void Board::falldownPiece(const Coord& _From, const Coord& _To) {
        //swap(_From, _To);
    }

    uint16_t Board::falldownColumn(uint16_t _Column) {
        std::queue<uint16_t> emptyCells;
        for (uint16_t y = 0; y < height_; y++) {
            const uint16_t & x = _Column;
            Coord coord(x, y);

            IPiece* piece = pieceAt(coord);

            if (!piece) {
                emptyCells.push(y);
            } else {
                if (!emptyCells.empty()) {
                    Coord target(x, emptyCells.front());
                    falldownPiece(coord, target);

                    emptyCells.push(y);
                    emptyCells.pop();
                }
            }
        }

        if (emptyCells.empty()) {
            return height();
        } else {
            return emptyCells.front();
        }
    }

    void Board::fillupColumn(const uint16_t _Column, const uint16_t firstEmptyCell) {
        uint16_t missingPieces = height() - firstEmptyCell;
        uint16_t newlyCreated = 0;
        while (missingPieces) {
            uint16_t y = height() - missingPieces--;
            Coord coord(_Column, y);
            IPiece* piece = generator_.createPiece(coord);
            setPiece(coord, piece);
            Coord ontop = Coord(_Column, height_ + newlyCreated++);
            falldownPiece(ontop, coord);
        }
    }

    void Board::refill() {
        uint16_t maxEmptyCells = 0;
        for (uint16_t column = 0; column < width_; column++) {
            uint16_t firstEmptyCell = falldownColumn(column);
            uint16_t columnEmptyCells = height() - firstEmptyCell;
            if (columnEmptyCells > 0) {
                fillupColumn(column, firstEmptyCell);
                if (columnEmptyCells > maxEmptyCells) {
                    maxEmptyCells = columnEmptyCells;
                }
            }
        }
    }

    bool Board::check() {
        return (checkDirection(HORIZONTAL) || checkDirection(VERTICAL));
    }

    bool Board::checkDirection(Direction _Direction) {
        std::set<IPiece*> removal;
        std::vector<IPiece*> container;

        const uint16_t & c_size = _Direction == HORIZONTAL ? width_ : height_;
        const uint16_t & r_size = _Direction == HORIZONTAL ? height_ : width_;
        container.reserve(c_size);

        for (uint16_t i = 0; i < r_size; i++) {
            uint16_t lastType = -1;
            uint16_t typeInRow = 0;

            container.clear();

            for (uint16_t j = 0; j < c_size; j++) {
                const uint16_t & x = _Direction == HORIZONTAL ? j : i;
                const uint16_t & y = _Direction == HORIZONTAL ? i : j;
                Coord coord(x, y);
                IPiece* piece = pieceAt(coord);
                if (piece->type() == lastType && j != 0) {
                    typeInRow++;
                    if (typeInRow > 3) {
                        container.push_back(piece);
                    } else if (typeInRow == 3) {
                        for (uint16_t before = 0; before < 3; before++) {
                            coord.x = _Direction == HORIZONTAL ? x - before : x;
                            coord.y = _Direction == HORIZONTAL ? y : y - before;
                            container.push_back(pieceAt(coord));
                        }
                    }
                } else {
                    if (!container.empty()) {
                        removal.insert(container.begin(), container.end());
                        container.clear();
                    }
                    typeInRow = 1;
                    lastType = piece->type();
                }
            }

            if (!container.empty()) {
                removal.insert(container.begin(), container.end());
                container.clear();
            }
        }

        for (IPiece* piece : removal) {
            remove(piece->getCoords());
        }

        return container.empty();
    }

} /* namespace match3 */

