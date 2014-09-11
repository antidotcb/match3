/*
 * Round.h
 *
 *  Created on: Sep 11, 2014
 *      Author: antidotcb
 */

#ifndef ROUND_H_
#define ROUND_H_

namespace match3 {

    class Round {
    public:
        Round() {
        }

        virtual ~Round() {
        }

    private:
        Board * board;
        Score * score;
        Time * time;
    };

} /* namespace match3 */

#endif /* ROUND_H_ */
