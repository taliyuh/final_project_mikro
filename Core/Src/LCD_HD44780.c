/**
  ******************************************************************************
  * @file     : LCD_HD44780.c
  * @author   : Mateusz Salamon   www.msalamon.pl
  * @version  : 1.0.0
  * @date     : Jul 11, 2018
  * @brief    : LCD screen driver
  *
  * The MIT License.
  ******************************************************************************
  */

#include "main.h"
#include "stm32f7xx_hal.h"
#include "gpio.h"


#include "LCD_HD44780.h"


/* Private Functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the LCD in 4-bit mode.
  * @retval None
  */
void LCD_Init(void)
{
    HAL_Delay(50); // Wait for LCD to power up

    // Initialize in 4-bit mode
    LCD_SendNibble(0x3);
    HAL_Delay(5);
    LCD_SendNibble(0x3);
    HAL_Delay(1);
    LCD_SendNibble(0x3);
    HAL_Delay(1);
    LCD_SendNibble(0x2);
    HAL_Delay(1);

    // Function set: 4-bit, 2 lines, 5x8 dots
    LCD_Command(0x28);
    // Display on, cursor off, blink off
    LCD_Command(0x0C);
    // Clear display
    LCD_Command(0x01);
    HAL_Delay(2);
    // Entry mode set: Increment cursor
    LCD_Command(0x06);
}

/**
  * @brief  Send a command byte to the LCD.
  * @param  cmd: Command byte
  * @retval None
  */
void LCD_Command(uint8_t cmd)
{
    // RS = 0 for command
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    LCD_SendByte(cmd);
    HAL_Delay(2);
}

/**
  * @brief  Send a data byte to the LCD.
  * @param  data: Data byte
  * @retval None
  */
void LCD_WriteData(uint8_t data)
{
    // RS = 1 for data
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
    LCD_SendByte(data);
    HAL_Delay(1);
}

/**
  * @brief  Send a 4-bit nibble to the LCD.
  * @param  nibble: Lower 4 bits of the byte
  * @retval None
  */
void LCD_SendNibble(uint8_t nibble)
{
    // Set data pins D4-D7
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (nibble & 0x1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (nibble & 0x2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (nibble & 0x4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (nibble & 0x8) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Toggle E pin
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
    HAL_Delay(1); // Enable pulse width
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Command processing time
}

/**
  * @brief  Send a full byte to the LCD by splitting into two nibbles.
  * @param  byte: Byte to send
  * @retval None
  */
void LCD_SendByte(uint8_t byte)
{
    // Send higher nibble
    LCD_SendNibble((byte >> 4) & 0x0F);
    // Send lower nibble
    LCD_SendNibble(byte & 0x0F);
}

/**
  * @brief  Clear the LCD display.
  * @retval None
  */
void LCD_Clear(void)
{
    LCD_Command(0x01);
}

/**
  * @brief  Set the cursor position on the LCD.
  * @param  row: Row number (0 or 1)
  * @param  col: Column number (0 to 15)
  * @retval None
  */
void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t address;
    if (row == 0)
        address = 0x80 + col;
    else
        address = 0xC0 + col;
    LCD_Command(address);
}

/**
  * @brief  Print a string on the LCD.
  * @param  str: Null-terminated string
  * @retval None
  */
void LCD_Print(char* str)
{
    while(*str)
    {
        LCD_WriteData(*str++);
    }
}
void LCD_CreateCustomChar(uint8_t* cg_char, uint8_t location)
{
    uint8_t i;

    // Set CGRAM address
    LCD_Command(0x40 + (location << 3));

    // Write character data
    for (i = 0; i < 8; i++) {
        LCD_WriteData(cg_char[i]);
    }
}


