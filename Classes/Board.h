#ifndef MATCH3_CLASSES_BOARD_H_
#define MATCH3_CLASSES_BOARD_H_

#include <cstdint>

#include "Piece.h"

namespace match3 {

    struct BoardSize {
        uint16_t width;
        uint16_t height;
        BoardSize(uint16_t _W, uint16_t _H);
    };

    class IBoard {
    public:
        virtual void add(const Coord& _Coord, IPiece* _Piece) = 0;
        virtual IPiece* pieceAt(const Coord& _Coord) const = 0;
        virtual IPiece* remove(const Coord& _Coord) = 0;
        virtual void swap(const Coord& _Pos1, const Coord& _Pos2) = 0;

        virtual uint16_t width() const = 0;
        virtual uint16_t height() const = 0;

        virtual ~IBoard() {
        }
    };

    class Board: public IBoard {
    public:
        static const BoardSize DefaultBoardSize;        // {8, 8};

        static Board* create(const IAbstractPieceFactory& _Factory, const BoardSize& _Size = DefaultBoardSize) {
            Board* pRet = new Board(_Factory, _Size);
            if (pRet && pRet->init()) {
                return pRet;
            } else {
                delete pRet;
                pRet = nullptr;
                return nullptr;
            }
        }

        virtual void add(const Coord& _Coord, IPiece* _Piece);
        virtual IPiece* pieceAt(const Coord& _Coord) const;
        virtual IPiece* remove(const Coord& _Coord);
        virtual void swap(const Coord& _Pos1, const Coord& _Pos2);

        virtual uint16_t width() const;
        virtual uint16_t height() const;

        virtual ~Board();

    protected:
        Board(const IAbstractPieceFactory& _Factory, const BoardSize& _Size);

        virtual bool init();
        virtual void cleanup();

        virtual void setPiece(const Coord& _Coord, IPiece* _Piece);
        virtual bool initCell(const Coord& _Coord);

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
