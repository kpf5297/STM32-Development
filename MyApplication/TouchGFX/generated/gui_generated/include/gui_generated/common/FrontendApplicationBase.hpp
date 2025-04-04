/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#ifndef FRONTENDAPPLICATIONBASE_HPP
#define FRONTENDAPPLICATIONBASE_HPP

#include <mvp/MVPApplication.hpp>
#include <gui/model/Model.hpp>

class FrontendHeap;

class FrontendApplicationBase : public touchgfx::MVPApplication
{
public:
    FrontendApplicationBase(Model& m, FrontendHeap& heap);
    virtual ~FrontendApplicationBase() { }

    virtual void changeToStartScreen()
    {
        gotoMenuScreenScreenNoTransition();
    }

    // MenuScreen
    void gotoMenuScreenScreenNoTransition();

    void gotoMenuScreenScreenBlockTransition();

    void gotoMenuScreenScreenCoverTransitionWest();

    // InstrOutScreen
    void gotoInstrOutScreenScreenSlideTransitionEast();

    // ControlScreen
    void gotoControlScreenScreenWipeTransitionNorth();

protected:
    touchgfx::Callback<FrontendApplicationBase> transitionCallback;
    FrontendHeap& frontendHeap;
    Model& model;

    // MenuScreen
    void gotoMenuScreenScreenNoTransitionImpl();

    void gotoMenuScreenScreenBlockTransitionImpl();

    void gotoMenuScreenScreenCoverTransitionWestImpl();

    // InstrOutScreen
    void gotoInstrOutScreenScreenSlideTransitionEastImpl();

    // ControlScreen
    void gotoControlScreenScreenWipeTransitionNorthImpl();
};

#endif // FRONTENDAPPLICATIONBASE_HPP
