#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include <2d/CCLayer.h>
#include <base/CCPlatformMacros.h>
#include <math/CCGeometry.h>
#include <math/Vec2.h>
#include <sys/types.h>

namespace cocos2d {
    class Label;
    class ProgressTimer;
}

namespace match3 {
    class GameBoardLayer;
    class Gameboard;
    class Piece;
    class Score;

    class GameLayer: public cocos2d::Layer {
    public:
        virtual bool init();
        virtual void cleanup();

        CREATE_FUNC(GameLayer)
                ;

    protected:
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
        Score * score_ = nullptr;
        Gameboard * gameboard_ = nullptr;
        GameBoardLayer* gameboardlayer = nullptr;

        cocos2d::ProgressTimer *timer_ = nullptr;
        cocos2d::Label *scoreLabel_ = nullptr;

        cocos2d::Size visibleSize_ = { 0, 0 };
        cocos2d::Vec2 origin_ = { 0, 0 };
        float delayTime = 0.0f;
        float time = 0.0f;

        bool enlargedScoreOnce_ = false;
        bool turnCompleted_ = true;
        bool highscore_ = false;
        bool gameEnded_ = false;

        static const float TotalGameTime;

        static const int DefaultBoardSize;

        static const int BackgroundLayerLevel;
        static const char* BackgroundTextureName;
        static const int ScoreLabelsLayerLevel = 1000;
        static const int UILayerLevel = 2000;
    };


}
#endif // __HELLOWORLD_SCENE_H__
