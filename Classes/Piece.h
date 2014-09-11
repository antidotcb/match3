/*
 * Piece.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef PIECE_H_
#define PIECE_H_

namespace match3 {

    class PieceColor {
    public:
        static const PieceColor * const red() {
            return &colors[ColorsIndexes::RED];
        }

        static const PieceColor * const blue() {
            return &colors[ColorsIndexes::BLUE];
        }

        static const PieceColor * const green() {
            return &colors[ColorsIndexes::GREEN];
        }

        static const PieceColor * const yellow() {
            return &colors[ColorsIndexes::YELLOW];
        }

        static const PieceColor * const purple() {
            return &colors[ColorsIndexes::PURPLE];
        }

        static const PieceColor * const random() {

        }

        bool operator ==(PieceColor & rhs) {
            return this->value == rhs.value;
        }
    private:
        enum ColorsIndexes {
            RED = 0,
            BLUE,
            GREEN,
            YELLOW,
            PURPLE,
            // add new colors above
            TOTAL_COLORS_NUMBER
        };

        PieceColor() :
                value(0)
        {
            if (!initialized_all_colors) {
                for (uint color = 0; color < ColorsIndexes::TOTAL_COLORS_NUMBER; color++) {
                    colors[color].value = color;
                }
                initialized_all_colors = true;
            }
        }

        static bool initialized_all_colors;
        static Piece colors[TOTAL_COLORS_NUMBER];

        uint value;
    };

    class Piece {
    public:
        Piece(PieceColor * _Color) :
                color(_Color) {
        }

        bool isSameColorAs (Piece & rhs) {
            return this->color == rhs.color;
        }

        virtual ~Piece() {
        }

    private:
        PieceColor *color;
    };

} /* namespace match3 */

#endif /* PIECE_H_ */
