#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include <base/CCPlatformMacros.h>
#include <math/CCGeometry.h>
#include <math/Vec2.h>

#include <memory>
#include "Score.h"

namespace cocos2d {
    class Label;
    class ProgressTimer;
}

namespace match3 {
    class Gameboard;
    class Gameboard;
    class IPiece;
    class Score;

    class GameLayer: public cocos2d::Layer {
    public:
        static cocos2d::Scene* wrapIntoScene();

        CREATE_FUNC(GameLayer)
                ;

    protected:
        virtual bool init();
        virtual ~GameLayer();

        bool onTouchBegan(cocos2d::Touch* _Touch, cocos2d::Event* _Event);
        void onTouchMoved(cocos2d::Touch* _Touch, cocos2d::Event* _Event);

        void onProgressTimer();
        void onTimeExpires();

        void startTurn();
        void endTurn();
        void endGame();

        void addInputDispatcher();
        void addBackground();
        void addScores();
        void addGameboard();
        void addProgressTimer();

    private:
        Score score_;
        Gameboard* gameboard_ = nullptr;

        cocos2d::ProgressTimer *timer_ = nullptr;
        cocos2d::Label *scoreLabel_ = nullptr;

        cocos2d::Size visibleSize_ = { 0, 0 };
        cocos2d::Vec2 origin_ = { 0, 0 };
        float delayTime = 0.0f;
        float time = 0.0f;

        bool enlargedScoreOnce_ = false;
        bool turnCompleted_ = true;
        bool highscore_ = true;
        bool gameEnded_ = false;

        static const float TotalGameTime;

        static const int BackgroundLayerLevel;
        static const char* BackgroundTextureName;
        static const int UILayerLevel = 2000;
    };

}
#endif // __HELLOWORLD_SCENE_H__
