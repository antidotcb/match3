/*
 * Board.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <cstdint>

namespace match3 {

    class Piece;

    class Board {
    public:
        Board(const uint8_t _Width = 8, const uint8_t _Height = 8) :
                width(_Width), height(_Height)
        {
            const uint8_t size = width * height;
            pieces = new Piece*[size];
        }

        const uint8_t getWidth() const {
            return width;
        }

        const uint8_t getHeight() const {
            return height;
        }

        virtual ~Board() {
            delete[] pieces;
        }

        bool isCollapsed() const;

        bool isSwapValid(Piece * from, Piece * where) const;

    protected:

    private:
        uint8_t width;
        uint8_t height;

        Piece** pieces;
    };

} /* namespace match3 */

#endif /* BOARD_H_ */
