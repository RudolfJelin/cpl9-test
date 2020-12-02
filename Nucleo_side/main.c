//
//  ******************************************************************************
//  @file    main.c
//  @author  CPL (Pavel Paces, based on STM examples)
//  @version V0.0
//  @date    01-October-2016
//  @brief   Serial line over ST-Link example
//  Nucleo STM32F401RE USART2 (Tx PA.2, Rx PA.3)
//
//  ******************************************************************************
//


#include "stm32f4xx.h"
#include "string.h"

// *******************************************************************************

//
//  @brief  Enable MCU internal connections to USART and GPIO
//
void RCC_Configuration(void)
{
  // --- System Clocks Configuration

  // USART2 clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  // GPIOA clock enable
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  // GPIOC clock enable
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
} // END void RCC_Configuration(void)

// *******************************************************************************

//
//  @brief  Input/output pins configuration
//
void GPIO_Configuration(void)
{
  // Init
	  GPIO_InitTypeDef GPIO_InitStructure;

	  // GPIO Configuration
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // PA.2 USART2_TX, PA.3 USART2_RX
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);
	  // Alternative Functions: Connect USART pins to AF
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	  // Init LED
	  GPIO_InitTypeDef GPIO_InitStructure_LED;
	  GPIO_InitStructure_LED.GPIO_Pin = GPIO_Pin_5;
	  GPIO_InitStructure_LED.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStructure_LED.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure_LED.GPIO_PuPd = GPIO_PuPd_UP;
	  GPIO_InitStructure_LED.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(GPIOA, &GPIO_InitStructure_LED);

	  // Init button
	  GPIO_InitTypeDef GPIO_InitStructure_Button;
	  GPIO_InitStructure_Button.GPIO_Pin = GPIO_Pin_13;
	  GPIO_InitStructure_Button.GPIO_Mode = GPIO_Mode_IN;
	  GPIO_InitStructure_Button.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure_Button.GPIO_PuPd = GPIO_PuPd_UP;
	  GPIO_InitStructure_Button.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(GPIOC, &GPIO_InitStructure_Button);

	  // Alternative Functions: Connect USART pins to AF
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	  } // END void GPIO_Configuration(void)

// *******************************************************************************

//
//  @brief  Baud rate settings
//
void USART2_Configuration(void)
{
  USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.USART_BaudRate 	= (9600)*3;
  USART_InitStructure.USART_WordLength 	= USART_WordLength_8b;
  USART_InitStructure.USART_StopBits 	= USART_StopBits_1;
  USART_InitStructure.USART_Parity 		= USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  USART_InitStructure.USART_Mode 		= USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART2, &USART_InitStructure);

  USART_Cmd(USART2, ENABLE);
} // END void USART2_Configuration(void)

// *******************************************************************************

//
//  @brief  The function sends characters until 0
//
void OutString(char *s)
{

	while(*s)
	{
		// Wait for Tx Empty
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		// Send Char
		USART_SendData(USART2, *s++);
	} // END while

} // END void OutString(char *s)

// *******************************************************************************
#define cBUFF_SIZE 255
//
//  @brief  Main loop
//
int main(void)
{
	RCC_Configuration();
	GPIO_Configuration();
	USART2_Configuration();

	uint16_t stateLED;

	char btInputBuffer[cBUFF_SIZE];
	int iItemsInBuffer = 0;

    while(1) // Don't want to exit
    {
        uint16_t Data;

        if( USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)  // If char in input
        {
            Data = USART_ReceiveData(USART2); // Collect char

            btInputBuffer[iItemsInBuffer++] = Data;

            if(iItemsInBuffer >= cBUFF_SIZE)
            {
            	iItemsInBuffer = 0;
            }

            if(iItemsInBuffer > 1){
            	if(btInputBuffer[iItemsInBuffer-2] == '\r' &&
            	   btInputBuffer[iItemsInBuffer-1] == '\n')
            	{
            		btInputBuffer[iItemsInBuffer-2] = 0;
            		iItemsInBuffer = 0;

            		if(strcmp("LED ON", btInputBuffer) == 0){
            			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
            		}

            		else if(strcmp("LED OFF", btInputBuffer) == 0){
            			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
            		}

            		else if(strcmp("BUTT:STATUS?", btInputBuffer) == 0){
            			if(GPIO_ReadInputDataBit( GPIOC, GPIO_Pin_13 ) == 0){
            				OutString("BUTTON:PRESSED\r\n");
            			}
            			else{
            				OutString("BUTTON:RELEASED\r\n");
            			}
            		}
            		else{ // unknown/custom command
            			OutString("Wrong command\r\n");
            		}
            	}
            }

        } // END if char in input

        // button controls
		if( GPIO_ReadInputDataBit( GPIOC, GPIO_Pin_13 ) == 0 ){
			if( stateLED == 0){
				GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET); // sets bit regardless of PC commands
				//OutString("LED ON\r\n");
				stateLED = 1;
			}
		}
		else{
			if( stateLED == 1){
				GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET); // sets bit regardless of PC commands
				//OutString("LED OFF\r\n");
				stateLED = 0;
			}
		} // END if-else button

    } // END while(1)

} // END main

// *******************************************************************************

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}
#endif
