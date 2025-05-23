/**
  ******************************************************************************
  * File Name          : TouchGFXGeneratedHAL.cpp
  ******************************************************************************
  * This file is generated by TouchGFX Generator 4.25.0. Please, do not edit!
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include <TouchGFXGeneratedHAL.hpp>
#include <touchgfx/hal/OSWrappers.hpp>
#include <gui/common/FrontendHeap.hpp>

#include <touchgfx/widgets/canvas/CWRVectorRenderer.hpp>

namespace touchgfx
{
VectorRenderer* VectorRenderer::getInstance()
{
    static CWRVectorRendererRGB565 renderer;

    return &renderer;
}
} // namespace touchgfx

#include "stm32f4xx.h"
#include "stm32f4xx_hal_ltdc.h"

using namespace touchgfx;

namespace
{
// Use the section "TouchGFX_Framebuffer" in the linker script to specify the placement of the buffer
// LOCATION_PRAGMA_NOLOAD("TouchGFX_Framebuffer")
// uint32_t frameBuf[(480 * 800 * 2 + 3) / 4] LOCATION_ATTRIBUTE_NOLOAD("TouchGFX_Framebuffer");
__attribute__((section(".TouchGFX_Framebuffer"), used))
static uint32_t frameBuf[(480 * 800 * 2 + 3) / 4];

static uint16_t lcd_int_active_line;
static uint16_t lcd_int_porch_line;
}

void TouchGFXGeneratedHAL::initialize()
{
    HAL::initialize();
    registerEventListener(*(Application::getInstance()));
    registerTaskDelayFunction(&OSWrappers::taskDelay);
    if (!setFrameRefreshStrategy(HAL::REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL))
    {
        while (1);
    }
    setFrameBufferStartAddresses((void*)frameBuf, (void*)0, (void*)0);
}

void TouchGFXGeneratedHAL::configureInterrupts()
{
    NVIC_SetPriority(DMA2D_IRQn, 9);
    NVIC_SetPriority(LTDC_IRQn, 9);
}

void TouchGFXGeneratedHAL::enableInterrupts()
{
    NVIC_EnableIRQ(DMA2D_IRQn);
    NVIC_EnableIRQ(LTDC_IRQn);
}

void TouchGFXGeneratedHAL::disableInterrupts()
{
    NVIC_DisableIRQ(DMA2D_IRQn);
    NVIC_DisableIRQ(LTDC_IRQn);
}

void TouchGFXGeneratedHAL::enableLCDControllerInterrupt()
{
    lcd_int_active_line = (LTDC->BPCR & LTDC_BPCR_AVBP_Msk) - 1;
    lcd_int_porch_line = (LTDC->AWCR & LTDC_AWCR_AAH_Msk) - 1;

    /* Sets the Line Interrupt position */
    LTDC->LIPCR = lcd_int_active_line;
    /* Line Interrupt Enable            */
    LTDC->IER |= LTDC_IER_LIE;
}

bool TouchGFXGeneratedHAL::beginFrame()
{
    return HAL::beginFrame();
}

void TouchGFXGeneratedHAL::endFrame()
{
    HAL::endFrame();
}

uint16_t* TouchGFXGeneratedHAL::getTFTFrameBuffer() const
{
    return (uint16_t*)LTDC_Layer1->CFBAR;
}

void TouchGFXGeneratedHAL::setTFTFrameBuffer(uint16_t* adr)
{
    LTDC_Layer1->CFBAR = (uint32_t)adr;

    /* Reload immediate */
    LTDC->SRCR = (uint32_t)LTDC_SRCR_IMR;
}

void TouchGFXGeneratedHAL::flushFrameBuffer(const touchgfx::Rect& rect)
{
    HAL::flushFrameBuffer(rect);
}

bool TouchGFXGeneratedHAL::blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes)
{
    return HAL::blockCopy(dest, src, numBytes);
}

uint16_t TouchGFXGeneratedHAL::getTFTCurrentLine()
{
    // This function only requires an implementation if single buffering
    // on LTDC display is being used (REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL).

    // The CPSR register (bits 15:0) specify current line of TFT controller.
    uint16_t curr = (uint16_t)(LTDC->CPSR & LTDC_CPSR_CYPOS_Msk);
    uint16_t backPorchY = (uint16_t)(LTDC->BPCR & LTDC_BPCR_AVBP_Msk) + 1;

    // The semantics of the getTFTCurrentLine() function is to return a value
    // in the range of 0-totalheight. If we are still in back porch area, return 0.
    return (curr < backPorchY) ? 0 : (curr - backPorchY);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
