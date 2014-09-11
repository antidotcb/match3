/*
 * Piece.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef PIECE_H_
#define PIECE_H_

#include <string>
#include <CCTexture2D.h>

//Director::getInstance()->getTextureCache()
namespace match3 {

    class PieceColor {
    public:
        bool operator ==(PieceColorManager & rhs) {
            return this->value == rhs.value;
        }
    private:
        static uint TotalColors = 0;
        PieceColor(const char * _ColorName, Texture2D * _Texture) {
        }
        std::string name;
        Texture2D * texture;
    };

    class PieceColorManager {
    public:
        static const PieceColor * const red() {
            return &available_colors[ColorsIndexes::RED];
        }

        static const PieceColor * const blue() {
            return &available_colors[ColorsIndexes::BLUE];
        }

        static const PieceColor * const green() {
            return &available_colors[ColorsIndexes::GREEN];
        }

        static const PieceColor * const yellow() {
            return &available_colors[ColorsIndexes::YELLOW];
        }

        static const PieceColor * const purple() {
            return &available_colors[ColorsIndexes::PURPLE];
        }

        static const PieceColor * const random() {
            return null;
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

        PieceColorManager() :
                value(0)
        {
            if (!initialized_all_colors) {
                for (uint color = 0; color < ColorsIndexes::TOTAL_COLORS_NUMBER; color++) {
                    available_colors[color].value = color;
                }
                initialized_all_colors = true;
            }
        }

        static bool initialized_all_colors;
        static Piece available_colors[TOTAL_COLORS_NUMBER]
        };

    class Piece {
    public:
        Piece(PieceColor * _Color) :
                color(_Color) {
        }

        bool isSameColorAs(const Piece & rhs) {
            return this->color == rhs.color;
        }

        virtual ~Piece() {
        }

    private:
        PieceColor *color;
    };

} /* namespace match3 */

#endif /* PIECE_H_ */
