#include "Piece.h"

#include <2d/CCSprite.h>
#include <base/CCDirector.h>
#include <math/CCGeometry.h>
#include <renderer/CCTextureCache.h>
#include <stdlib.h>

namespace match3 {
    USING_NS_CC;

    struct TextureConfig {
        uint32_t value;
        const char * texture;
    };

    static const TextureConfig config[] = {
            { 0, "" },
            { 1, "Blue.png" },
            { 2, "Green.png" },
            { 3, "Purple.png" },
            { 4, "Red.png" },
            { 5, "Yellow.png" }
    };

    PiecesManager* PiecesManager::instance_ = nullptr;

    PiecesManager::PiecesManager() {
    }

    IPiece* PiecesManager::createPiece(const Coord& _Position) const {
        auto piece = Piece::create(random(), _Position);
        return piece;
    }

    uint32_t PiecesManager::random() const {
        /* not compatible with android :(
         static std::default_random_engine generator;
         static std::uniform_int_distribution<int> distribution(0, );
         */
        uint32_t random = (rand() / (float) RAND_MAX * (textures_.size()-1)) + 1;
        return random;
    }

    Texture2D* PiecesManager::texture(uint16_t _Value) {
        auto it = textures_.find(_Value);
        return (it != textures_.end()) ? it->second : nullptr;
    }

    bool PiecesManager::loadTextures() {
        for (uint32_t i = 0; i < sizeof(config) / sizeof(TextureConfig); i++) {
            auto dir = Director::getInstance();
            textures_[config[i].value] = dir->getTextureCache()->addImage(config[i].texture);
        }

        return true;
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

    Piece::Piece(uint16_t _Type, const Coord& _Position) :
            coord_(_Position), type_(_Type) {
        ObjCount++;
        //CCLOGINFO("%s", __func__);
    }

    const Coord& Piece::getCoords() const {
        return coord_;
    }

    void Piece::setCoords(const Coord& _Position) {
        coord_ = _Position;
    }

    bool Piece::isSameTypeAs(const IPiece* _Piece) const {
        return this->type() == _Piece->type();
    }

    bool Piece::isNextTo(const IPiece* _Piece) const {
        if (this->getCoords().x == _Piece->getCoords().x) {
            return (this->getCoords().y - _Piece->getCoords().y == 1)
                    || (this->getCoords().y - _Piece->getCoords().y == -1);
        } else
        if (this->getCoords().y == _Piece->getCoords().y) {
            return (this->getCoords().x - _Piece->getCoords().x == 1)
                    || (this->getCoords().x - _Piece->getCoords().x == -1);
        } else {
            return false;
        }
    }

    uint16_t Piece::type() const {
        return type_;
    }

    bool Piece::init() {
        sprite_ = Sprite::create();
        auto dir = PiecesManager::getInstance();
        auto tex = dir->texture(type());

        if (tex) {
            sprite_->setTexture(tex);

            Rect rect(0, 0,
                    tex->getContentSize().width,
                    tex->getContentSize().height);

            sprite_->setTextureRect(rect);
        } else {
            CCLOGERROR("No texture for type %d", type());
        }
        return true;
    }

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

    void Piece::setSprite(cocos2d::Sprite* _Sprite) {
        sprite_ = _Sprite;
    }

    cocos2d::Sprite* Piece::sprite() const {
        return sprite_;
    }

    uint32_t Piece::ObjCount = 0;
} /* namespace match3 */

