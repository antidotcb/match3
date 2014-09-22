#ifndef MATCH3_CLASSES_PIECE_H_
#define MATCH3_CLASSES_PIECE_H_

#include <base/CCPlatformMacros.h>
#include <renderer/CCTexture2D.h>
#include <cstdint>
#include <vector>

namespace cocos2d {
    class Sprite;
} /* namespace cocos2d */

namespace match3 {
    struct Coord {
        uint16_t x;
        uint16_t y;
        Coord(uint16_t X, uint16_t Y) : x(X), y(Y) {}
    };

    class IPiece {
    public:
        virtual uint16_t type() const = 0;

        virtual const Coord& getCoords() const = 0;
        virtual void setCoords(const Coord& _Position) = 0;

        virtual cocos2d::Sprite* sprite() const = 0;
        virtual void setSprite(cocos2d::Sprite* _Sprite)= 0;

        virtual bool isSameTypeAs(const IPiece* _Piece) const = 0;
        virtual bool isNextTo(const IPiece* _Piece) const = 0;

        virtual ~IPiece() {
            CCLOG("IPiece::~IPiece()");
        }
    };

    class Piece: public IPiece {
    public:
        static uint32_t ObjCount;

        static Piece* create(uint16_t _Type, const Coord& _Position);

        virtual uint16_t type() const;

        virtual const Coord& getCoords() const;
        virtual void setCoords(const Coord& _Position);

        virtual cocos2d::Sprite* sprite() const;
        virtual void setSprite(cocos2d::Sprite* _Sprite);

        virtual bool isSameTypeAs(const IPiece* _Piece) const;
        virtual bool isNextTo(const IPiece* _Piece) const;

        virtual ~Piece() {
            ObjCount--;
            //CCLOGINFO("%s", __func__);
        }

    protected:
        Piece(uint16_t _Type, const Coord& _Position);

        virtual bool init();

    private:
        Coord coord_;
        uint16_t type_ = 0;
        cocos2d::Sprite* sprite_ = nullptr;
    };

    class IAbstractPieceFactory {
    public:
        virtual IPiece* createPiece(const Coord& _Position) const = 0;
        virtual ~IAbstractPieceFactory() {
        }
    };

    class PiecesManager: public IAbstractPieceFactory {
    public:
        static PiecesManager* getInstance();
        static void destroyInstance();

        virtual IPiece* createPiece(const Coord& _Position) const;
        cocos2d::Texture2D* texture(uint16_t _Value);
        bool loadTextures();

    protected:
        uint32_t random() const;

    private:
        PiecesManager();
        DISALLOW_COPY_AND_ASSIGN(PiecesManager);

        typedef std::map<uint16_t, cocos2d::Texture2D*> TextureMap;
        TextureMap textures_;

        static PiecesManager * instance_;
    };

} /* namespace match3 */

#endif /* MATCH3_CLASSES_PIECE_H_ */
