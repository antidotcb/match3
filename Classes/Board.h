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
    class Gameboard: public cocos2d::Node {
    public:
        struct Size {
            uint16_t width;
            uint16_t height;
        };

        static Gameboard* create(const Size& _Size);
        virtual void swap(const Coord& _Pos1, const Coord& _Pos2);

        virtual ~Gameboard();

        Piece* getPiece(Coord _Coord);
        void remove(Piece* piece);

        Coord wolrd2coord(const cocos2d::Vec2& _TouchPos);
        cocos2d::Vec2 coord2world(const Coord & _Coord);

        float fillup(IAbstractPieceFactory* _PieceFactory, bool animate = true);
        void getResultsOfLastFill(std::list<std::vector<Piece*> > &_Container);

        bool check();
        void getResultsOfLastCheck(std::list<std::vector<Piece*> > &_Container);

        void lock();
        void unlock();
        bool locked() const;

        static const float FalldownSpeed;        // 0.2f

    protected:
        cocos2d::Vec2 center() const;

        Coord local2coord(const cocos2d::Vec2 & _Pos);
        cocos2d::Vec2 coord2local(const Coord & _Coord);

        Gameboard(const Size& _Size);

        virtual bool init();

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
        uint16_t height_;
        //cocos2d::Layer* layer_;
        bool locked_;
        static const uint8_t FgSpriteLevel = 100;
        static const uint8_t BgSpriteLevel = 50;
        static const float CellSize;
        static const float HalfCellSize;

        static const char* BgSpriteTextureName;        // "background.png"
    };

} /* namespace match3 */

#endif /* BOARD_H_ */
