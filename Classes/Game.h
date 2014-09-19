#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include <base/CCPlatformMacros.h>
#include <math/CCGeometry.h>
#include <math/Vec2.h>
#include <sys/types.h>

#include "Score.h"

namespace cocos2d {
    class Label;
    class ProgressTimer;
}


namespace match3 {
    class Gameboard;
    class Piece;
    class Score;

    class GameLayer: public cocos2d::Layer {
    public:
        static cocos2d::Scene* wrapIntoScene();

        virtual ~GameLayer();
        virtual bool init();

        CREATE_FUNC(GameLayer)
        ;

    protected:
        void afterSwap(Node* _Node);
        void afterSwapBack(Node* _Node);

        void addInputDispatcher();
        void addBackground();
        void addScores();
        void addGameboard();
        void addProgressTimer();
        void addScoreLabel(uint _Score, const cocos2d::Vec2 & _Position);
        void addSomeStars(const cocos2d::Vec2 & _Position);

        void select(Piece* _Piece);
        void deselect();

        void dissapear(Piece* _Piece);

        void swapPieces(Piece* _First, Piece* _Second);

        void cleanup();

        bool onTouchBegan(cocos2d::Touch* _Touch, cocos2d::Event* _Event);
        void onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event);

    private:
        void onProgressTimer();
        void onTimeExpires();

        Score * score_;

        Gameboard * gameboard_ = nullptr;
        cocos2d::ProgressTimer *timer_ = nullptr;
        cocos2d::Label *scoreLabel_ = nullptr;

        cocos2d::Size visibleSize_ = {0,0};
        cocos2d::Vec2 origin_ = {0,0};

        Piece *selected_ = nullptr;
        Node *firstArrived_ = nullptr;

        float delayTime = 0.0f;
        float time = 0.0f;

        static const float TotalGameTime;
        static const float FastSpeed;
        static const float SlowSpeed;
        static const float DissapearSpeed;

        static const float FastSpeedLL;
        static const float SlowSpeedLL;

        static const int HighlightActionsTag;

        static const int DefaultBoardSize;

        static const int BackgroundLayerLevel;
        static const char* BackgroundTextureName;
        static const int ScoreLabelsLayerLevel = 1000;
        static const int UILayerLevel = 2000;

    };

}
#endif // __HELLOWORLD_SCENE_H__
