#include "Piece.h"

#include <2d/CCSprite.h>
#include <base/CCDirector.h>
#include <math/CCGeometry.h>
#include <renderer/CCTextureCache.h>
#include <stdlib.h>

namespace match3 {
    USING_NS_CC;

    Texture2D* PiecesManager::texture(uint32_t _Value) {
        return _Value < textures_.size() ? textures_[_Value] : nullptr;
    }

    uint32_t PiecesManager::random() const {
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
            auto dir = Director::getInstance();
            textures_.push_back(dir->getTextureCache()->addImage(config[i].texture));
        }

        return true;
    }

    Piece::Piece(uint16_t _Type, const Coord& _Position) :
            coord_(_Position), type_(_Type) {
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
            return (this->getCoords().y - _Piece->getCoords().y == 1) || (this->getCoords().y - _Piece->getCoords().y == -1);
        } else
        if (this->getCoords().y == _Piece->getCoords().y) {
            return (this->getCoords().x - _Piece->getCoords().x == 1) || (this->getCoords().x - _Piece->getCoords().x == -1);
        } else {
            return false;
        }
    }

    uint16_t Piece::type() const {
        return type_;
    }

    Piece* PiecesManager::createPiece(const Coord& _Position) const {
        auto piece = SpritePiece::create(random(), _Position);
        return piece;
    }

    bool SpritePiece::init() {
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

    Sprite* SpritePiece::sprite() const {
        return sprite_;
    }

    PiecesManager* PiecesManager::instance_ = 0;

    PiecesManager::PiecesManager() {
    }

    SpritePiece* SpritePiece::create(uint16_t _Type, const Coord& _Position) {
        SpritePiece *pRet = new SpritePiece(_Type, _Position);
        if (pRet && pRet->init()) {
            return pRet;
        } else {
            delete pRet;
            pRet = NULL;
            return NULL;
        }
    }

    SpritePiece::SpritePiece(uint16_t _Type, const Coord& _Position) :
            Piece(_Type, _Position) {
    }

} /* namespace match3 */

