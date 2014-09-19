/*
 * Piece.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#include "Piece.h"

#include <2d/CCSprite.h>
#include <base/CCDirector.h>
#include <base/CCPlatformMacros.h>
#include <math/CCGeometry.h>
#include <renderer/CCTextureCache.h>
#include <stddef.h>
#include <stdlib.h>
#include <cstdint>

USING_NS_CC;

namespace match3 {

    cocos2d::Texture2D* PiecesManager::texture(uint32_t _Value) {
        return _Value < textures_.size() ? textures_[_Value] : nullptr;
    }

    uint32_t PiecesManager::random() {
        /* not compatible with android :(
         static std::default_random_engine generator;
         static std::uniform_int_distribution<int> distribution(0, );
         */
        uint32_t random = rand() / (float) RAND_MAX * (textures_.size());
        return random;
    }

    void PiecesManager::destroyInstance() {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    PiecesManager* PiecesManager::getInstance() {
        if (!instance_) {
            instance_ = new PiecesManager();
        }
        return instance_;
    }

    struct TextureConfig {
        uint32_t value;
        const char * name;
        const char * texture;
    };

    static const TextureConfig config[] = {
            { 1, "blue", "Blue.png" },
            { 2, "green", "Green.png" },
            { 3, "purple", "Purple.png" },
            { 4, "red", "Red.png" },
            { 5, "yellow", "Yellow.png" }
    };

    bool PiecesManager::loadTextures() {
        for (uint32_t i = 0; i < sizeof(config) / sizeof(TextureConfig); i++) {
            auto dir = cocos2d::Director::getInstance();
            textures_.push_back(dir->getTextureCache()->addImage(config[i].texture));
        }

        return true;
    }

    Piece::Piece(uint16_t _Type, const Coord& _Position) :
            position_(_Position), type_(_Type) {
    }

    const Coord& Piece::coord() const {
        return position_;
    }

    void Piece::setPosition(const Coord& _Position) {
        position_ = _Position;
    }

    bool Piece::isSameTypeAs(const Piece* _Piece) const {
        return this->type() == _Piece->type();
    }

    bool Piece::isNextTo(const Piece* _Piece) const {
        if (this->coord().x == _Piece->coord().x) {
            return (this->coord().y - _Piece->coord().y == 1) || (this->coord().y - _Piece->coord().y == -1);
        } else
        if (this->coord().y == _Piece->coord().y) {
            return (this->coord().x - _Piece->coord().x == 1) || (this->coord().x - _Piece->coord().x == -1);
        } else {
            return false;
        }
    }

    uint16_t Piece::type() const {
        return type_;
    }

    Piece* PiecesManager::createPiece(const Coord& _Position) {
        auto piece = Piece::create(random(), _Position);
        return piece;
    }

    bool Piece::init() {
        sprite_ = Sprite::create();
        auto dir = PiecesManager::getInstance();
        auto tex = dir->texture(type());

        // TODO:
//        if (!tex) {
//            return false;
//        }

        sprite_->setTexture(tex);

        Rect rect(0, 0,
                tex->getContentSize().width,
                tex->getContentSize().height);

        sprite_->setTextureRect(rect);
        return true;
    }

    cocos2d::Sprite* Piece::sprite() const {
        return sprite_;
    }

    PiecesManager* PiecesManager::instance_ = 0;

    PiecesManager::PiecesManager() {
    }

//    PiecesManager::PiecesManager(PiecesManager&) {
//    }
//
//    PiecesManager& PiecesManager::operator =(const PiecesManager&) {
//        return *this;
//    }

    Piece* Piece::create(uint16_t _Type, const Coord& _Position) {
        Piece *pRet = new Piece(_Type, _Position);
        if (pRet && pRet->init()) {
            return pRet;
        } else {
            delete pRet;
            pRet = NULL;
            return NULL;
        }
    }

} /* namespace match3 */

