#pragma once

#include <stdint.h>

#define MMIO32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

//rst and clk
#define RCC_AHB1ENR MMIO32(0x40023830UL)
#define RCC_APB1ENR MMIO32(0x40023840UL)
#define RCC_GPIOAEN (1UL << 0)
#define RCC_USART2EN (1UL << 17)

//gpioa
#define GPIOA_MODER MMIO32(0x40020000UL)
#define GPIOA_OTYPER MMIO32(0x40020004UL)
#define GPIOA_ODR MMIO32(0x40020014UL)
#define GPIOA_AFRL MMIO32(0x40020020UL)

//usart2
#define USART2_SR MMIO32(0x40004400UL)
#define USART2_DR MMIO32(0x40004404UL)
#define USART2_BRR MMIO32(0x40004408UL)
#define USART2_CR1 MMIO32(0x4000440CUL)

#define USART_SR_RXNE (1UL << 5)
#define USART_SR_TXE (1UL << 7)
#define USART_CR1_RE (1UL << 2)
#define USART_CR1_TE (1UL << 3)
#define USART_CR1_UE (1UL << 13)
#define USART_CR1_RXNEIE (1UL << 5)

//interrupts
#define NVIC_ISER1 MMIO32(0xE000E104UL)
#define USART2_IRQN 38U

#define LED_PIN 5U