/*
 * common.h
 *
 *  Created on: Sep 13, 2014
 *      Author: antidotcb
 */

#ifndef COMMON_H_
#define COMMON_H_

#undef COCOS2D_DEBUG
#define COCOS2D_DEBUG 3

#include <cocos2d.h>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <functional>
//#include <base/CCDirector.h>
//#include <base/CCPlatformMacros.h>
//#include <CCFileUtils.h>
//#include <renderer/CCTextureCache.h>
namespace match3 {

    struct Coord {
        uint16_t X;
        uint16_t Y;

        Coord(uint16_t _X, uint16_t _Y) :
                X(_X), Y(_Y) {}
    };

//    class IPiece {
//    public:
//        virtual const Coord& position() const = 0;
//        virtual void setPosition(const Coord& _Position) = 0;
//        virtual bool isSameTypeAs(const IPiece* _Piece) const = 0;
//        virtual bool isNextTo(const IPiece* _Piece) const = 0;
//        virtual uint16_t type() const = 0;
//        virtual ~IPiece() {
//        }
//    };

    class Piece;

    class IAbstractPieceFactory {
    public:
        virtual Piece* createPiece() = 0;
        virtual ~IAbstractPieceFactory() {
        }
    };
}

#endif /* COMMON_H_ */

