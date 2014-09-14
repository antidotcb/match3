/*
 * Piece.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef PIECE_H_
#define PIECE_H_

#include "cocos2d.h"
#include <map>
#include <string>

namespace match3 {

    class PiecesManager;

    class PieceColor {
    public:
        const std::string& name() const;
        cocos2d::Texture2D* texture() const;
        uint32_t value() const;

        bool operator ==(const PieceColor& _Rhs) const;

    private:
        PieceColor(uint32_t _Value, const char* _ColorName, cocos2d::Texture2D* _Texture);

        uint32_t value_;
        std::string name_;
        cocos2d::Texture2D * texture_;

        friend class PiecesManager;
    };

    class PiecesManager {
    public:
        PieceColor* random();
        PieceColor* color(uint32_t _Value);

        bool loadPieces(const char* _Filename);

        static PiecesManager* getInstance();
        static void destroyInstance();

    private:
        PiecesManager();
        PiecesManager(PiecesManager&);
        PiecesManager& operator =(PiecesManager&);

        typedef std::map<int, PieceColor*> ColorsMap;
        ColorsMap colors_;

        static PiecesManager * instance_;
    };

    class Piece {
    public:
        Piece(PieceColor* _Color);
        virtual ~Piece();

        PieceColor* color() const { return color_; }
        bool isSameColorAs(const Piece& _Rhs) const;

    private:
        PieceColor *color_;
    };

} /* namespace match3 */

#endif /* PIECE_H_ */
