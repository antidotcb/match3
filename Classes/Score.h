#ifndef MATCH3_CLASSES_SCORE_H_
#define MATCH3_CLASSES_SCORE_H_

#include <2d/CCActionInstant.h>
#include <2d/CCActionInterval.h>
#include <2d/CCLabel.h>
#include <2d/CCLayer.h>
#include <base/ccMacros.h>
#include <cstdint>
#include <sstream>
#include <string>

namespace match3 {

    class Score: public cocos2d::Layer {
    public:
        virtual void setLabel(cocos2d::Label* _Label) {
            label_ = _Label;
        }

        virtual void newTurn() {
            group_bonus_ = 0;
            sequence_bonus_ = 0;
        }

        virtual uint32_t addGroup(uint32_t _GroupSize) {
            group_bonus_ += GroupScore;

            uint32_t pieceScore = PieceScore * _GroupSize + group_bonus_ + sequence_bonus_;

            score_ += pieceScore * _GroupSize;

            updateLabel();

            return pieceScore;
        }

        virtual void newSequence(uint32_t _GroupsCount) {
            sequence_bonus_ = 0;
            if (_GroupsCount > MinSequenceCountForBonus) {
                sequence_bonus_ = (_GroupsCount - MinSequenceCountForBonus) * SequenceScore;
            }
        }

        virtual ~Score() {
        }
    protected:
        virtual void updateLabel() {
            if (label_) {
                if (labelScore_ < score_) {
                    label_->stopActionByTag(UpdateTag);
                    auto diff = score_ - labelScore_;
                    auto step = diff / (float) StepsCount;

                    auto update = cocos2d::Sequence::create(
                            cocos2d::DelayTime::create(1.f / (float) StepsCount),
                            cocos2d::CallFunc::create(CC_CALLBACK_0(Score::step, this)),
                            nullptr);

                    auto repeat = cocos2d::Repeat::create(update, StepsCount);
                    step_ = step;
                    label_->runAction(repeat);
                } else {
                    label_->stopActionByTag(UpdateTag);
                    label_->setString(convertScoreToText(score_));
                    labelScore_ = score_;
                }
            }
        }

        void step() {
            if (label_) {
                labelScore_ += step_;
                if (labelScore_ > score_) {
                    updateLabel();
                }
                label_->setString(convertScoreToText(labelScore_));
            }
        }

        virtual std::string & convertScoreToText(uint32_t _Score) {
            converter_.clear();
            converter_ << _Score;
            converter_ >> scoreText_;
            return scoreText_;
        }

    private:
        std::stringstream converter_;
        std::string scoreText_;

        cocos2d::Label* label_ = nullptr;

        uint32_t score_ = 0;
        uint32_t labelScore_ = 0;
        uint32_t step_ = 0;
        uint32_t group_bonus_ = 0;
        uint32_t sequence_bonus_ = 0;

        static const uint32_t GroupScore = 5;
        static const uint32_t PieceScore = 10;
        static const uint32_t SequenceScore = 50;
        static const uint32_t MinSequenceCountForBonus = 2;
        static const uint32_t StepsCount = 20;
        static const uint32_t UpdateTag = 0xABCD;
    };

} /* namespace match3 */

#endif /* MATCH3_CLASSES_SCORE_H_ */
