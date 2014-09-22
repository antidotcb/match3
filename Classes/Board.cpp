#include "Board.h"

#include <base/CCPlatformMacros.h>
#include <queue>
#include <set>
#include <vector>

namespace match3 {
    USING_NS_CC;

    const BoardSize Board::DefaultBoardSize = { 8, 8 };

    BoardSize::BoardSize(uint16_t _W, uint16_t _H) :
            width(_W), height(_H) {
    }

    Board::Board(const IAbstractPieceFactory& _Factory, const BoardSize& _Size) :
            generator_(_Factory), width_(_Size.width), height_(_Size.height) {
    }

    bool Board::init() {
        if (board_) {
            cleanup();
        }

        if (!width_ || !height_) {
            return false;
        }

        board_ = new BoardRow[height_];
        if (!board_) {
            return false;
        }

        memset(board_, 0, sizeof(BoardRow) * height_);
        for (uint16_t y = 0; y < height_; y++) {
            board_[y] = new BoardPiece[width_];
            if (!board_[y]) {
                cleanup();
                return false;
            }
            memset(board_[y], 0, sizeof(BoardPiece) * width_);
            for (uint16_t x = 0; x < width_; x++) {
                Coord coord(x, y);
                if (!initCell(coord)) {
                    cleanup();
                    return false;
                }
            }
        }

        return true;
    }

    bool Board::initCell(const Coord& _Coord) {
        setPiece(_Coord, nullptr);
        return true;
    }

    void Board::cleanup() {
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
    }

    Board::~Board() {
        cleanup();
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

    IPiece* Board::remove(const Coord & _Coord) {
        IPiece* piece = pieceAt(_Coord);
        board_[_Coord.y][_Coord.x] = nullptr;
        return piece;
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

    void Board::add(const Coord& _Coord, IPiece* _Piece) {
        setPiece(_Coord, _Piece);
    }

} /* namespace match3 */
