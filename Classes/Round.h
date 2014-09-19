/*
 * Round.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef ROUND_H_
#define ROUND_H_

#include "common.h"

namespace match3 {

    class Board;
    class Time;
    class Score;

    class BasicRound {
    public:
        BasicRound();
        virtual ~BasicRound() {
        }

        void startNewRound();
        bool isRoundFinished();
        uint16_t resultScore();
        uint16_t timeLeft();
        private:
        Board * board;
        Score * score;
        Time * time;
    };

} /* namespace match3 */

#endif /* ROUND_H_ */
