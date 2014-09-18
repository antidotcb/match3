/*
 * Board.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "common.h"

namespace match3 {
    class Gameboard {
    public:
        struct Size {
            uint16_t width;
            uint16_t height;
        };

        static Gameboard* create(const cocos2d::Vec2 _Position,const Size& _Size, IAbstractPieceFactory* _Factory, cocos2d::Layer* _Layer);
        virtual void swap(const Coord& _Pos1, const Coord& _Pos2);

        virtual ~Gameboard();

        Piece* getPiece(Coord _Coord);
        void remove(Piece* piece);

        Coord screenToCell(const cocos2d::Vec2 _TouchPos);
        cocos2d::Vec2 cellToScreen(const Coord & _Coord);

        float fillup();
        void getResultsOfLastFill(std::list<std::vector<Piece*> > &_Container);

        bool check();
        void getResultsOfLastCheck(std::list<std::vector<Piece*> > &_Container);

        void lock();
        void unlock();
        bool locked() const;

        static const float FalldownSpeed;        // 0.2f

    protected:
        Gameboard(const Size& _Size, IAbstractPieceFactory* _Factory, cocos2d::Layer * _Layer);

        virtual bool init(const cocos2d::Vec2 & _Position);

        void checkDirection(bool _Horizontal);
        void cleanup();
        bool validate();

        void setPiece(Coord _Coord, Piece* _Piece);

    private:
        typedef Piece* BoardPiece;
        typedef BoardPiece* BoardRow;

        std::list<std::vector<Piece*>> piecesToRemove_;
        std::vector<Piece*> newPieces_;

        BoardRow* board_;
        uint16_t width_;
        uint16_t heigth_;
        IAbstractPieceFactory* factory_;
        cocos2d::Layer* layer_;
        bool locked_;
        cocos2d::Vec2 origin_;

        std::vector<cocos2d::Sprite*> sprites_;

        static const cocos2d::Vec2 DefaultOrigin;

        static const uint8_t FgSpriteLevel = 100;
        static const uint8_t BgSpriteLevel = 50;
        static const uint8_t CellSize = 40;
        static const uint8_t CellPadding = 0;



        static const char* BgSpriteTextureName;        // "background.png"
    };

} /* namespace match3 */

#endif /* BOARD_H_ */
