#ifndef  MATCH3_CLASSES_APP_DELEGATE_H_
#define  MATCH3_CLASSES_APP_DELEGATE_H_

#include <CCApplication.h>

namespace cocos2d {
    class Scene;
} /* namespace cocos2d */

namespace match3 {

    class App: private cocos2d::Application {
    public:
        static cocos2d::Scene* wrapIntoScene(cocos2d::Layer* _Layer);

        App();
        virtual ~App();

        virtual bool applicationDidFinishLaunching();
        virtual void applicationDidEnterBackground();
        virtual void applicationWillEnterForeground();

    private:
        void adaptResolution(float _DesignW, float _DesignH);
    };

} /* namespace match3 */

#endif // MATCH3_CLASSES_APP_DELEGATE_H_
