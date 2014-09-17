#include "HelloWorldScene.h"
#include "Piece.h"
#include "Board.h"
#include <SimpleAudioEngine.h>

USING_NS_CC;

namespace match3 {
    const float GameLayer::FastSpeed = .25f;
    const float GameLayer::SlowSpeed = .5f;
    const float GameLayer::FastSpeedLL = GameLayer::FastSpeed + (GameLayer::FastSpeed/10.0f);
    const float GameLayer::SlowSpeedLL = GameLayer::SlowSpeed + (GameLayer::SlowSpeed/10.0f);

    void GameLayer::addBackground()
    {
        Sprite* sprite = Sprite::create("bg.png");
        sprite->setPosition(Vec2(visibleSize_.width / 2 + origin_.x,
                visibleSize_.height / 2 + origin_.y));
        this->addChild(sprite, -100);
    }

    bool GameLayer::addGameboard() {
        Gameboard::Size boardSize = { DefaultBoardSize, DefaultBoardSize };
        Vec2 position;
        gameboard_ = Gameboard::create(position, boardSize, PiecesManager::getInstance(), this);

        if (!gameboard_) {
            CCLOGERROR("Can't create gameboard.");
            return false;
        }

        return true;
    }

    Scene* GameLayer::wrapIntoScene() {
        auto scene = Scene::create();        // 'scene' is an autorelease object
        auto layer = GameLayer::create();        // 'layer' is an autorelease object
        scene->addChild(layer);
        return scene;
    }

    GameLayer::GameLayer() :
            gameboard_(0), selectedPiece_(0), firstArrived_(0) {
        visibleSize_ = Size { 0, 0 };
        origin_ = Vec2(0, 0);
    }

    void GameLayer::addInputDispatcher()
    {
        EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();
        listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
        //listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);
        //listener->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnded, this);
        //listener->onTouchCancelled = CC_CALLBACK_2(GameLayer::onTouchCancelled, this);
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    }

    bool GameLayer::init() {

        if (!Layer::init()) {
            return false;
        }

        visibleSize_ = Director::getInstance()->getVisibleSize();
        origin_ = Director::getInstance()->getVisibleOrigin();

        // add "Play" splash screen
        addBackground();

        if (!addGameboard()) {
            CCLOGERROR("Can't init gameboard.");
            return false;
        }

        addInputDispatcher();

        CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music.mp3");
        CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("move.aif");
        //CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("music.mp3", true);

        return true;
    }

    bool GameLayer::onTouchBegan(cocos2d::Touch *_Touch, cocos2d::Event *_Event) {
        Vec2 touch_pos = _Touch->getLocation();
        CCLOGINFO("Touch [X=%f , Y=%f]", touch_pos.x, touch_pos.y);
        Vec2 window_pos = Director::getInstance()->convertToGL(touch_pos);
        CCLOGINFO("Window [X=%f , Y=%f]", touch_pos.x, touch_pos.y);
        Coord coord = gameboard_->screenToCell(touch_pos);
        CCLOGINFO("Coord [X=%d , Y=%d]", coord.X, coord.Y);
        Piece* piece = gameboard_->getPiece(coord);

        if (gameboard_->locked() || !piece) {
            return true;
        }

        CCLOG("piece color: %s", piece->color()->name().c_str());

        if (selectedPiece_ && selectedPiece_->isNextTo(piece)) {
            gameboard_->lock();
            Piece* first = selectedPiece_;
            deselectPiece();
            swapPieces(first, piece, CC_CALLBACK_1(GameLayer::checkSwapValid, this));
        } else {
            selectPiece(piece);
        }

        return true;
    }

    void GameLayer::selectPiece(Piece* _Piece) {
        if (selectedPiece_) {
            deselectPiece();
        }

        Sequence* selectSequence;
        FadeTo* fadeIn = FadeTo::create(SlowSpeed, 192);
        FadeTo* fadeOut = FadeTo::create(SlowSpeed, 255);

        selectSequence = Sequence::createWithTwoActions(
                fadeIn,
                fadeOut);

        RepeatForever *pulsation = RepeatForever::create(selectSequence);
        pulsation->setTag(HighlightActionsTag);

        _Piece->sprite()->runAction(pulsation);

        selectedPiece_ = _Piece;
    }

    void GameLayer::deselectPiece() {
        if (!selectedPiece_) {
            return;
        }

        //selectedPiece_->sprite()->stopAction(selectedPulseAction_);
        selectedPiece_->sprite()->stopActionByTag(HighlightActionsTag);

        FadeTo* fadeOut = FadeTo::create(.0f, 255);
        selectedPiece_->sprite()->runAction(fadeOut);

        selectedPiece_ = nullptr;
    }

    void GameLayer::cleanup()
    {
        if (gameboard_) {
            delete gameboard_;
            gameboard_ = 0;
        }
    }

    GameLayer::~GameLayer()
    {
        cleanup();
    }

//void GameLayer::menuCloseCallback(Ref* pSender) {
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
//    MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
//    return;
//#endif
//
//    Director::getInstance()->end();
//
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//    exit(0);
//#endif
//}

    void GameLayer::checkSwapValid(Piece* _Piece) {
        if (firstArrived_ == nullptr) {
            CCLOG("checkSwapValid: First arrived, proceed further.");
            firstArrived_ = _Piece;
            return;
        }

        CCLOG("checkSwapValid: Second arrived, finally can check.");

        if (gameboard_->check()) {

            // TODO: add animation of destroyed items

            std::list<std::vector<Piece*> > removePieces;
            gameboard_->getResultsOfLastCheck(removePieces);

            for (auto vec : removePieces) {
                std::stringstream ss;
                for (Piece* piece : vec) {
                    ss << piece->color()->name() << "[" << piece->position().X << "," << piece->position().Y << "] ";
                    gameboard_->removePiece(piece);
                    //delete piece;
                }

                CCLOG("Found %d pieces in row with following colors: %s", vec.size(), ss.str().c_str());
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("remove.wav");
            }

            gameboard_->fillup();

            gameboard_->unlock();
        } else {
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("move.aif");
            swapPieces(firstArrived_, _Piece, CC_CALLBACK_1(GameLayer::swapBack, this));
        }

        firstArrived_ = nullptr;
    }

    void GameLayer::swapBack(Piece* _Piece) {
        if (firstArrived_ == nullptr) {
            CCLOG("swapBack: First arrived, proceed further.");
            firstArrived_ = _Piece;
            return;
        }

        firstArrived_ = nullptr;

        gameboard_->unlock();
        CCLOG("swapBack: Second arrived, finally can unlock board.");
    }

    void GameLayer::swapPieces(Piece* _First, Piece* _Second, const std::function<void(Piece*)> &cb) {
        Sprite* spriteA = _First->sprite();
        Sprite* spriteB = _Second->sprite();

        Coord posA = _First->position();
        Coord posB = _Second->position();

        CallFunc *checkFuncA = CallFunc::create(std::bind(cb, _First));
        CallFunc *checkFuncB = CallFunc::create(std::bind(cb, _Second));

        MoveTo *moveA = MoveTo::create(FastSpeed, gameboard_->cellToScreen(posB));
        MoveTo *moveB = MoveTo::create(FastSpeed, gameboard_->cellToScreen(posA));

        Sequence *sequenceA = Sequence::createWithTwoActions(moveA, checkFuncA);
        Sequence *sequenceB = Sequence::createWithTwoActions(moveB, checkFuncB);

        spriteA->runAction(sequenceA);
        spriteB->runAction(sequenceB);

        gameboard_->swap(posA, posB);
    }

}
