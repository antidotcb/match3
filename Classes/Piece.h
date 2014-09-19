/*
 * Piece.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef PIECE_H_
#define PIECE_H_

#include "common.h"

namespace match3 {

    class BasicPiece {
    public:
        BasicPiece(uint16_t _Type, const Coord& _Position);
        const virtual Coord& coord() const;
        virtual void setPosition(const Coord& _Position);
        virtual bool isSameTypeAs(const BasicPiece* _Piece) const;
        virtual bool isNextTo(const BasicPiece* _Piece) const;
        virtual uint16_t type() const;
        virtual ~BasicPiece() {
        }
    private:
        Coord position_;
        uint16_t type_;
    };

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

    class Piece: public BasicPiece {
    public:
        static Piece* create(PieceColor* _Color);

        cocos2d::Sprite* sprite() const;

        virtual bool init();
        virtual void autorelease();

        virtual void setPosition(const Coord& _Position);
        virtual uint16_t type() const;

        PieceColor* color() const;

        virtual ~Piece();
        protected:

    private:
        Piece(PieceColor* _Color);

        PieceColor *color_;
        cocos2d::Sprite* sprite_;
    };

    class PiecesManager: public IAbstractPieceFactory {
    public:
        PieceColor* random();
        PieceColor* color(uint32_t _Value);

        virtual Piece* createPiece();

        bool loadPieces(const char* _Filename);

        static PiecesManager* getInstance();
        static void destroyInstance();

    private:
        PiecesManager();
        PiecesManager(PiecesManager&);
        PiecesManager& operator =(const PiecesManager&);

        typedef std::map<int, PieceColor*> ColorsMap;
        ColorsMap colors_;

        static PiecesManager * instance_;
    };

} /* namespace match3 */

#endif /* PIECE_H_ */
