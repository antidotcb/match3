/*
 * Piece.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef PIECE_H_
#define PIECE_H_

#include <cstdint>
#include <map>

#include "common.h"

namespace match3 {

    class Piece: public cocos2d::Node {
    public:
        static Piece* create(uint16_t _Type, const Coord& _Position);

        virtual bool init();

        virtual uint16_t type() const;
        virtual const Coord& coord() const;
        virtual cocos2d::Sprite* sprite() const;

        virtual void setPosition(const Coord& _Position);
        virtual bool isSameTypeAs(const Piece* _Piece) const;
        virtual bool isNextTo(const Piece* _Piece) const;

        virtual ~Piece() {}
    private:
        Piece(uint16_t _Type, const Coord& _Position);

        Coord position_;
        uint16_t type_;
        cocos2d::Sprite* sprite_ = nullptr;
    };

    class PiecesManager: public IAbstractPieceFactory {
    public:
        static PiecesManager* getInstance();
        static void destroyInstance();

        virtual Piece* createPiece(const Coord& _Position);
        cocos2d::Texture2D* texture(uint32_t _Value);
        bool loadTextures();

    protected:
        uint32_t random();

    private:
        PiecesManager();
        PiecesManager(PiecesManager&);
        PiecesManager& operator =(const PiecesManager&);

        typedef std::vector<cocos2d::Texture2D*> TextureMap;
        TextureMap textures_;

        static PiecesManager * instance_;
    };

} /* namespace match3 */

#endif /* PIECE_H_ */
