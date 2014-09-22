#ifndef MATCH3_CLASSES_MENU_H_
#define MATCH3_CLASSES_MENU_H_

#include <2d/CCLayer.h>
#include <base/CCPlatformMacros.h>

namespace match3 {

    class MenuLayer: public cocos2d::Layer
    {
    public:
        static cocos2d::Scene* wrapIntoScene();


        virtual bool init();

        void menuClose(cocos2d::Ref* pSender);
        void menuPlay(cocos2d::Ref* pSender);
        void menuHighscores(cocos2d::Ref* pSender);
        void menuSettings(cocos2d::Ref* pSender);

        CREATE_FUNC(MenuLayer);
    };

} /* namespace match3 */

#endif /* MATCH3_CLASSES_MENU_H_ */
