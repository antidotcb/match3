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

namespace match3 {

    class Coord {
    public:
        uint16_t x;
        uint16_t y;

        Coord(uint16_t X, uint16_t Y) :
                x(X), y(Y) {
        }
    };
}

#endif /* COMMON_H_ */

