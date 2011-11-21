// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2011, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------
/*
 * WARNING: This file is generated automatically, do not edit!
 * Please modify the corresponding *.in file instead and rebuild this file. 
 */
// ----------------------------------------------------------------------------

#include "../gpio.hpp"
#include "../device.h"

#include "timer_5.hpp"

#if !defined (STM32F10X_LD) && !defined (STM32F10X_MD) 


// ----------------------------------------------------------------------------
void
xpcc::stm32::Timer5::enable()
{
	// enable clock
	RCC->APB1ENR  |=  RCC_APB1ENR_TIM5EN;
	
	// reset timer
	RCC->APB1RSTR |=  RCC_APB1RSTR_TIM5RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM5RST;
}

void
xpcc::stm32::Timer5::disable()
{
	// disable clock
	RCC->APB1ENR &= ~RCC_APB1ENR_TIM5EN;
	
	TIM5->CR1 = 0;
	TIM5->DIER = 0;
	TIM5->CCER = 0;
}

// ----------------------------------------------------------------------------
void
xpcc::stm32::Timer5::setMode(Mode mode)
{
	// disable timer
	TIM5->CR1 = 0;
	TIM5->CR2 = 0;
	
	if (mode == ENCODER)
	{
		// SMS[2:0] = 011
		//   Encoder mode 3 - Counter counts up/down on both TI1FP1 and TI2FP2
		//   edges depending on the level of the other input.
		TIM5->SMCR = TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;
		
		setPrescaler(1);
	}
	else {
		// ARR Register is buffered, only Under/Overflow generates update interrupt
		TIM5->CR1 = TIM_CR1_ARPE | TIM_CR1_URS | mode;
		
		// Slave mode disabled
		TIM5->SMCR = 0;
	}
}

// ----------------------------------------------------------------------------
uint16_t
xpcc::stm32::Timer5::setPeriod(uint32_t microseconds, bool autoApply)
{
	// This will be inaccurate for non-smooth frequencies (last six digits
	// unequal to zero)
	uint32_t cycles = microseconds * (F_CPU / 1000000UL);
	uint16_t prescaler = (cycles + 65535) / 65536;	// always round up
	uint16_t overflow = cycles / prescaler;
	
	overflow = overflow - 1;	// e.g. 36000 cycles are from 0 to 35999
	
	setPrescaler(prescaler);
	setOverflow(overflow);
	
	if (autoApply) {
		// Generate Update Event to apply the new settings for ARR
		TIM5->EGR |= TIM_EGR_UG;
	}
	
	return overflow;
}

// ----------------------------------------------------------------------------
void
xpcc::stm32::Timer5::configureOutputChannel(uint32_t channel,
		OutputCompareMode mode, uint16_t compareValue)
{
	channel -= 1;	// 1..4 -> 0..3
	
	// disable output
	TIM5->CCER &= ~((TIM_CCER_CC1P | TIM_CCER_CC1E) << (channel * 4));
	
	setCompareValue(channel, compareValue);
	
	// enable preload (the compare value is loaded at each update event)
	uint32_t flags = mode | TIM_CCMR1_OC1PE;
	
	if (channel <= 1)
	{
		uint32_t offset = 8 * channel;
		
		flags <<= offset;
		flags |= TIM5->CCMR1 & ~(0xff << offset);
		
		TIM5->CCMR1 = flags;
	}
	else {
		uint32_t offset = 8 * (channel - 2);
		
		flags <<= offset;
		flags |= TIM5->CCMR2 & ~(0xff << offset);
		
		TIM5->CCMR2 = flags; 
	}
	
	if (mode != OUTPUT_INACTIVE) {
		TIM5->CCER |= (TIM_CCER_CC1E) << (channel * 4);
	}
}

// ----------------------------------------------------------------------------
void
xpcc::stm32::Timer5::enableInterrupt(Interrupt interrupt)
{
	// register IRQ at the NVIC
	NVIC_EnableIRQ(TIM5_IRQn);
	
	TIM5->DIER |= interrupt;
}


#endif
