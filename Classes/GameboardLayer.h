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
    class Piece;
    class Score;
} /* namespace match3 */

namespace match3 {

    class GameBoardLayer: public cocos2d::Layer {
    public:
        virtual bool init();

        virtual void cleanup();

        static GameBoardLayer* create(Gameboard* _Gameboard, Score* _Score);

        bool onTouchBegan(cocos2d::Touch* _Touch, cocos2d::Event* _Event);
        void onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event);

    protected:
        GameBoardLayer(Gameboard* _Gameboard, Score* _Score);

        void afterSwap(Node* _Node);
        void afterSwapBack(Node* _Node);

        void addBackground();

        void addScoreLabel(uint _Score, const cocos2d::Vec2 & _Position);
        void addSomeEffects(const cocos2d::Vec2 & _Position);

        void select(Piece* _Piece);
        void deselect();
        void dissapear(Piece* _Piece);

        void swapPieces(Piece* _First, Piece* _Second);

    private:
        Gameboard * gameboard_ = nullptr;
        Score * score_ = nullptr;

        Piece *selected_ = nullptr;
        Node *firstArrived_ = nullptr;

        static const float FastSpeed;
        static const float SlowSpeed;
        static const float DissapearSpeed;

        static const float FastSpeedLL;
        static const float SlowSpeedLL;

        static const int HighlightActionsTag;

        static const int ScoreLabelsLayerLevel = 1000;
    };

} /* namespace match3 */

#endif /* GAMEBOARDLAYER_H_ */
