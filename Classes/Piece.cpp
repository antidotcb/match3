/*
 * Piece.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#include "Piece.h"

USING_NS_CC;

namespace match3 {

    const std::string& PieceColor::name() const {
        return name_;
    }

    cocos2d::Texture2D* PieceColor::texture() const {
        return texture_;
    }

    inline uint32_t PieceColor::value() const {
        return value_;
    }

    bool PieceColor::operator ==(const PieceColor& _Rhs) const {
        return this->value_ == _Rhs.value_;
    }

    PieceColor::PieceColor(uint32_t _Value, const char* _ColorName, cocos2d::Texture2D* _Texture) :
            value_(_Value), name_(_ColorName), texture_(_Texture) {
    }

    PieceColor* PiecesManager::color(uint32_t _Value) {
        return colors_.find(_Value) == colors_.end() ? nullptr : colors_[_Value];
    }

    PieceColor* PiecesManager::random() {
        static std::default_random_engine generator;
        static std::uniform_int_distribution<int> distribution(0, colors_.size() - 1);

        ColorsMap::const_iterator item = colors_.begin();
        size_t random = distribution(generator);
        std::advance(item, random);

        return item->second;
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

    bool PiecesManager::loadPieces(const char *_Filename) {
        std::string colors_file;

        cocos2d::FileUtils* futils = cocos2d::FileUtils::getInstance();
        if (!futils) {
            CCLOGERROR("Cannot create FileUtils instance.");
            return false;
        }

        colors_file = futils->fullPathForFilename(_Filename);

        std::ifstream input_file;
        input_file.open(colors_file.c_str());
        if (!input_file.is_open()) {
            CCLOGERROR("Cannot open file %s", colors_file.c_str());
            return false;
        }

        colors_.clear();
        while (!input_file.eof()) {
            uint32_t value;

            std::string name;
            std::string texture_name;

            input_file >> value;
            input_file >> name;
            input_file >> texture_name;

            CCLOG("value=%d, name=%s, texture=%s", value, name.c_str(), texture_name.c_str());

            if ((this->color(value) != nullptr) || (name.empty() || texture_name.empty())) {
                CCLOGERROR("Colors file contains duplicates");

                colors_.clear();
                input_file.close();

                return false;
            }

            cocos2d::Director* dir = cocos2d::Director::getInstance();
            cocos2d::Texture2D * tex = dir->getTextureCache()->addImage(texture_name);

            colors_[value] = new PieceColor(value, name.c_str(), tex);

            input_file.ignore();
        }

        input_file.close();

        return true;
    }

    Piece::Piece(PieceColor* _Color) :
            BasicPiece(_Color->value(), Coord(0, 0)), color_(_Color), sprite_(0) {
    }

    BasicPiece::BasicPiece(uint16_t _Type, const Coord& _Position) :
            position_(_Position), type_(_Type) {
    }

    const Coord& BasicPiece::position() const {
        return position_;
    }

    void BasicPiece::setPosition(const Coord& _Position) {
        position_ = _Position;
    }

    bool BasicPiece::isSameTypeAs(const BasicPiece* _Piece) const {
        return this->type() == _Piece->type();
    }

    bool BasicPiece::isNextTo(const BasicPiece* _Piece) const {
        if (this->position().X == _Piece->position().X) {
            return (this->position().Y - _Piece->position().Y == 1) || (this->position().Y - _Piece->position().Y == -1);
        } else
        if (this->position().Y == _Piece->position().Y) {
            return (this->position().X - _Piece->position().X == 1) || (this->position().X - _Piece->position().X == -1);
        } else {
            return false;
        }
    }

    uint16_t BasicPiece::type() const {
        return type_;
    }

    Piece* PiecesManager::createPiece() {
        PieceColor* color = random();
        return Piece::create(color);
    }

    uint16_t Piece::type() const {
        return color_->value();
    }

    PieceColor* Piece::color() const {
        return color_;
    }

    bool Piece::init() {
        sprite_ = Sprite::create();
        cocos2d::Texture2D* tex = color_->texture();
        sprite_->setTexture(tex);
        Rect rect;
        rect.size.width = tex->getContentSize().width;
        rect.size.height = tex->getContentSize().height;
        sprite_->setTextureRect(rect);
        // TODO: size

        return true;
    }

    cocos2d::Sprite* Piece::sprite() const {
        return sprite_;
    }

    PiecesManager* PiecesManager::instance_ = 0;

    PiecesManager::PiecesManager() {
    }

    PiecesManager::PiecesManager(PiecesManager&) {
    }

    PiecesManager& PiecesManager::operator =(const PiecesManager&) {
        return *this;
    }

    void Piece::setPosition(const Coord& _Position)
            {
        BasicPiece::setPosition(_Position);
        //TODO: Setup position on screen
    }

    Piece* Piece::create(PieceColor* _Color) {
        Piece *pRet = new Piece(_Color);
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = NULL;
            return NULL;
        }
    }

    void Piece::autorelease()
    {
    }

    Piece::~Piece()
    {
        autorelease();
    }

} /* namespace match3 */

