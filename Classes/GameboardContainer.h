/*
 * GameboardContainer.h
 *
 *  Created on: Sep 21, 2014
 *      Author: antidotcb
 */

#ifndef MATCH3_CLASSES_BOARD_H_
#define MATCH3_CLASSES_BOARD_H_

#include <cstdint>

namespace match3 {
    struct Coord;
    class IAbstractPieceFactory;
    class IPiece;
} /* namespace match3 */

namespace match3 {
    struct BoardSize {
        uint16_t width;
        uint16_t height;
        BoardSize(uint16_t _W, uint16_t _H);
    };

    class Board {
    public:
        Board(const BoardSize& _Size, const IAbstractPieceFactory& _Factory);

        virtual IPiece* pieceAt(const Coord& _Coord) const;
        virtual void remove(const Coord& _Coord);
        virtual void swap(const Coord& _Pos1, const Coord& _Pos2);

        virtual void refill();
        virtual bool check();

        virtual uint16_t width() const;
        virtual uint16_t height() const;

        virtual ~Board();

    protected:
        enum Direction {
            HORIZONTAL, VERTICAL
        };

        virtual bool init();
        virtual void clean();
        virtual bool checkDirection(const Direction _Direction);
        virtual void setPiece(const Coord& _Coord, IPiece* _Piece);

        virtual bool initCell(const Coord& _Coord);
        virtual void fillupColumn(const uint16_t _Column, const uint16_t firstEmptyCell);
        virtual uint16_t falldownColumn(const uint16_t _Column);

        virtual void falldownPiece(const Coord& _From, const Coord& _To);

    private:
        typedef IPiece* BoardPiece;
        typedef BoardPiece* BoardRow;

        const IAbstractPieceFactory& generator_;

        BoardRow* board_ = nullptr;
        uint16_t width_ = 0;
        uint16_t height_ = 0;
    };

} /* namespace match3 */

#endif /* MATCH3_CLASSES_BOARD_H_ */
