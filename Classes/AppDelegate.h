#ifndef  MATCH3_CLASSES_APP_DELEGATE_H_
#define  MATCH3_CLASSES_APP_DELEGATE_H_
#include <cocos2d.h>

    class AppDelegate: private cocos2d::Application {
    public:


        AppDelegate();
        virtual ~AppDelegate();

        virtual bool applicationDidFinishLaunching();
        virtual void applicationDidEnterBackground();
        virtual void applicationWillEnterForeground();

    private:
        void adaptResolution(float _DesignW, float _DesignH);
    };


#endif // MATCH3_CLASSES_APP_DELEGATE_H_
