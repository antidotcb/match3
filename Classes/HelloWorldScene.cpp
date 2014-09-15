#include "HelloWorldScene.h"
#include "Piece.h"

#include <functional>

USING_NS_CC;

Scene* HelloWorld::createScene() {
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init() {
    //////////////////////////////
    // 1. super init first
    if (!Layer::init()) {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 3. add your codes below...

//    // add a label shows "Hello World"
//    // create and initialize a label
//    auto label = LabelTTF::create("Hello World", "Arial", 24);
//
//    // position the label on the center of the screen
//    label->setPosition(
//            Vec2(origin.x + visibleSize.width / 2,
//                    origin.y + visibleSize.height
//                            - label->getContentSize().height));
//
//    // add the label as a child to this layer
//    this->addChild(label, 1);

    match3::Piece* board[8][8];

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            match3::PiecesManager* pm = match3::PiecesManager::getInstance();
            match3::PieceColor* color = pm->random();
            match3::Piece* piece = new match3::Piece(color);
            board[i][j] = piece;
            Sprite *pieceSprite = Sprite::create("Blue.png");
            pieceSprite->setTexture(piece->color()->texture());
            pieceSprite->setPosition(
                    Vec2(visibleSize.width / 2 + origin.x + ((i - 4) * 42),
                            visibleSize.height / 2 + origin.y + ((j - 4) * 42)));
            this->addChild(pieceSprite, 0);

            Sprite *bgSprite;
            if (i <= 3) {
                bgSprite = Sprite::create("background_green.png");
            } else if (i <= 5) {
                bgSprite = Sprite::create("background_blue.png");
            } else {
                bgSprite = Sprite::create("background.png");
            }
            //sprite->setTexture(piece->color()->texture());

            bgSprite->setPosition(
                    Vec2(visibleSize.width / 2 + origin.x + ((i - 4) * 42),
                            visibleSize.height / 2 + origin.y + ((j - 4) * 42)));
            this->addChild(bgSprite, -50);

        }
    }

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("bg.png");

    // position the sprite on the center of the screen
    sprite->setPosition(
            Vec2(visibleSize.width / 2 + origin.x,
                    visibleSize.height / 2 + origin.y));

    //sprite->runAction(Liquid::create(2, Size(32, 24), 1, 20));

    // add the sprite as a child to this layer
    this->addChild(sprite, -100);

    return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
