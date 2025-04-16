#ifndef SPLASH_SCREENVIEW_HPP
#define SPLASH_SCREENVIEW_HPP

#include <gui_generated/splash_screen_screen/Splash_ScreenViewBase.hpp>
#include <gui/splash_screen_screen/Splash_ScreenPresenter.hpp>

class Splash_ScreenView : public Splash_ScreenViewBase
{
public:
    Splash_ScreenView();
    virtual ~Splash_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // SPLASH_SCREENVIEW_HPP
