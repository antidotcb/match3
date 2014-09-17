#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "common.h"

namespace match3 {
    class Gameboard;
    class Piece;

    class GameLayer: public cocos2d::Layer {
    public:
        // there's no 'id' in cpp, so we recommend returning the class instance pointer
        static cocos2d::Scene* wrapIntoScene();

        // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
        virtual bool init();

        // a selector callback
        // void menuCloseCallback(cocos2d::Ref* pSender);

        // implement the "static create()" method manually
        CREATE_FUNC(GameLayer)
        ;

        GameLayer();
        virtual ~GameLayer();

    protected:
        void checkSwapValid(Piece* _Piece);
        void swapBack(Piece* _Piece);

        void addBackground();
        bool addGameboard();

        void selectPiece(Piece* _Piece);
        void deselectPiece();
        void swapPieces(Piece* _First, Piece* _Second, const std::function<void(Piece*)> &cb);

        void cleanup();

        bool onTouchBegan(cocos2d::Touch* _Touch, cocos2d::Event* _Event);
        void addInputDispatcher();

        //void onTouchMoved(Touch*, Event*);
        //void onTouchEnded(Touch*, Event*);
        //void onTouchCancel(Touch*, Event*);

    private:
        Gameboard * gameboard_;

        cocos2d::Size visibleSize_;
        cocos2d::Vec2 origin_;

        //cocos2d::Action *selectedPulseAction_;
        Piece *selectedPiece_;
        Piece *firstArrived_;

        static const int DefaultBoardSize = 8;
        static const float FastSpeed;        // 0.2f
        static const float SlowSpeed;        // 0.5f
        static const float FastSpeedLL;        // 0.22f
        static const float SlowSpeedLL;        // 0.55f

        static const int HighlightActionsTag = 50;
    };

}
#endif // __HELLOWORLD_SCENE_H__
