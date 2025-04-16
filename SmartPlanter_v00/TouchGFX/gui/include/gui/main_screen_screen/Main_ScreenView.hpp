#ifndef MAIN_SCREENVIEW_HPP
#define MAIN_SCREENVIEW_HPP

#include <gui_generated/main_screen_screen/Main_ScreenViewBase.hpp>
#include <gui/main_screen_screen/Main_ScreenPresenter.hpp>

class Main_ScreenView : public Main_ScreenViewBase
{
public:
    Main_ScreenView();
    virtual ~Main_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void light00_clicked();
protected:
};

#endif // MAIN_SCREENVIEW_HPP
