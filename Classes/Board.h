/*
 * Board.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef BOARD_H_
#define BOARD_H_

namespace match3 {

    class Board {
    public:
        Board(const uint _Width, const uint _Height):
            width(_Width), height(_Height)
        {
            const uint size = width * height;
            pieces = new Piece* [size];
        }

        const uint getWidth() const {
            return width;
        }

        const uint getHeight() const {
            return height;
        }

        virtual ~Board() {
            delete []  pieces;
        }

        bool isCollapsed() const;

        bool isSwapValid(Piece * from, Piece * where) const;

    protected:

    private:
        uint width;
        uint height;

        Piece** pieces;
    };

} /* namespace match3 */

#endif /* BOARD_H_ */
