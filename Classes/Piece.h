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
        const std::string & name() const {
            return name_;
        }

        Texture2D * texture() const {
            return texture_;
        }

        bool operator ==(PieceColorManager & rhs) {
            return this->value_ == rhs.value_;
        }
    private:
        PieceColor(const char * _ColorName, Texture2D * _Texture):
            value_(TotalColors++), name_(_ColorName), texture_(_Texture)
        {}

        static uint TotalColors = 0;
        uint value_;

        std::string name_;
        Texture2D * texture_;
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
