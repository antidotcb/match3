/*
 * Gameboardlayer.h
 *
 *  Created on: Sep 21, 2014
 *      Author: antidotcb
 */

#ifndef GAMEBOARDLAYER_H_
#define GAMEBOARDLAYER_H_

#include <2d/CCLayer.h>
#include <math/Vec2.h>
#include <sys/types.h>

namespace match3 {
    class Gameboard;
    class IPiece;
    class Score;
    class IBoard;
    class IPiece;
    struct Coord;
    class IAbstractPieceFactory;
} /* namespace match3 */

namespace match3 {

    class Gameboard: public cocos2d::Layer {
    public:
        virtual bool init();
        virtual bool isActive() {
            return active_;
        }

        virtual void cleanup();

        static Gameboard* create(Score* _Score);

        bool onTouchBegan(cocos2d::Touch* _Touch, cocos2d::Event* _Event);
        void onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event);

    protected:
        enum Direction {
            HORIZONTAL, VERTICAL
        };

        virtual bool check();
        virtual void checkDirection(const Direction _Direction);

        virtual uint16_t refill();
        virtual void fillupColumn(const uint16_t _Column, const uint16_t firstEmptyCell);
        virtual uint16_t falldownColumn(const uint16_t _Column);
        virtual void falldownPiece(const Coord& _From, const Coord& _To);

        Gameboard(Score* _Score);

        void afterSwap(Node* _Node);
        void afterSwapBack(Node* _Node);

        void addBackground();

        void addScoreLabel(uint _Score, const cocos2d::Vec2 & _Position);
        void addSomeEffects(const cocos2d::Vec2 & _Position);

        void select(IPiece* _Piece);
        void deselect();
        void dissapear(IPiece* _Piece);

        void swapPieces(IPiece* _First, IPiece* _Second);

        cocos2d::Vec2 center() const;
        cocos2d::Vec2 coord2local(const Coord& _Coord) const;
        cocos2d::Vec2 coord2world(const Coord& _Coord) const;
        Coord wolrd2coord(const cocos2d::Vec2& _TouchPos) const;
        Coord local2coord(const cocos2d::Vec2& _Pos) const;

        void lock();
        void unlock();
        bool locked() const;

    private:
        std::list<std::vector<IPiece*> > checkResults_;

        IBoard * board_ = nullptr;
        Score * score_ = nullptr;

        IPiece *selected_ = nullptr;
        Node *firstArrived_ = nullptr;

        bool locked_ = false;
        bool animate_ = false;
        bool active_ = false;

        static const float FalldownSpeed;

        static const float FastSpeed;
        static const float SlowSpeed;
        static const float DissapearSpeed;

        static const float FastSpeedLL;
        static const float SlowSpeedLL;

        static const uint8_t HighlightTag = 64;

        static const uint8_t ForegroundLevel = 127;
        static const uint8_t BackgroundLevel = 0;
        static const uint8_t TopmostLevel = 255;

        static const float HalfCellSize;
        static const float CellSize;

        static const float ElasticModifier;

        static const char* BgSpriteTextureName;
    };

} /* namespace match3 */

#endif /* GAMEBOARDLAYER_H_ */
