#include "stm32f1xx.h" // CMSIS Header file

void USART1_Init(void) {
    RCC->APB2ENR |= (1 << 2) | (1 << 14); // Enable GPIOA and USART1

    GPIOA->CRH &= ~(0xF << 4); // Clear PA9 settings
    GPIOA->CRH |= (0xB << 4);  // Set PA9 as AF Push-Pull, 50MHz

    USART1->BRR = 0x0271; // Baud Rate = 115200
    USART1->CR1 |= (1 << 3) | (1 << 13); // Enable TX, USART
}

void USART1_SendChar(char c) {
    while (!(USART1->SR & (1 << 7))); // Wait for TXE
    USART1->DR = c;
}

void USART1_SendString(char *str) {
    while (*str) USART1_SendChar(*str++);
}

int main(void) {
    USART1_Init();
    while (1) {
        USART1_SendString("Hello from Mac Silicon!\r\n");
        for (int i = 0; i < 1000000; i++); // Basic delay
    }
}
