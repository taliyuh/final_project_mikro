

#ifndef LCD_HD44780_H_
#define LCD_HD44780_H_
/* Define to prevent recursive inclusion -------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

/* LCD Pin Definitions */
#define LCD_RS_Pin       GPIO_PIN_6
#define LCD_RS_GPIO_Port GPIOC

#define LCD_E_Pin        GPIO_PIN_15
#define LCD_E_GPIO_Port  GPIOB

#define LCD_D4_Pin       GPIO_PIN_12
#define LCD_D4_GPIO_Port GPIOB

#define LCD_D5_Pin       GPIO_PIN_15
#define LCD_D5_GPIO_Port GPIOA

#define LCD_D6_Pin       GPIO_PIN_15
#define LCD_D6_GPIO_Port GPIOD

#define LCD_D7_Pin       GPIO_PIN_12
#define LCD_D7_GPIO_Port GPIOF

/* Function Prototypes -------------------------------------------------------*/
void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_SendNibble(uint8_t nibble);
void LCD_SendByte(uint8_t byte);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(char* str);

#ifdef __cplusplus
}
#endif


#endif /* LCD_HD44780_H_ */
