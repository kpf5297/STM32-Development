#include <gui/main_screen_screen/Main_ScreenView.hpp>
#include "main.h"

#ifndef SIMULATOR
	#include "stm32f4xx_hal.h"
#endif

Main_ScreenView::Main_ScreenView()
{

}

void Main_ScreenView::setupScreen()
{
    Main_ScreenViewBase::setupScreen();
}

void Main_ScreenView::tearDownScreen()
{
    Main_ScreenViewBase::tearDownScreen();
}

void Main_ScreenView::light00_clicked() {

#ifndef SIMULATOR
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
#endif

}
