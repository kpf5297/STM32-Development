#ifndef SPLASH_SCREENPRESENTER_HPP
#define SPLASH_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Splash_ScreenView;

class Splash_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Splash_ScreenPresenter(Splash_ScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Splash_ScreenPresenter() {}

private:
    Splash_ScreenPresenter();

    Splash_ScreenView& view;
};

#endif // SPLASH_SCREENPRESENTER_HPP
