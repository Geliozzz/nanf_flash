//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include "stm32f4xx_hal.h"
#include "fsmc_nand_if.h"


// ----------------------------------------------------------------------------
//
// Semihosting STM32F4 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

void SystemClock_Config(void);

uint8_t dataToWrite[512];
uint8_t dataToRead[512];

int
main(int argc, char* argv[])
{
  // At this stage the system clock should have already been configured
  // at high speed.
	uint16_t i;
	SystemClock_Config();
//	Flash_Test();
	NAND_Init();

	for (i = 0; i < 512; i++)
	    {
	        dataToWrite[i] = i + 1;
	        dataToRead[i] = 0x00;
	    }

	  //  NAND_Write(10, dataToWrite, 512);
	  //  NAND_Read(10, dataToRead, 512);
	NAND_ADDRESS m_address;
	m_address.Zone = 0;
	m_address.Page = 2;
	m_address.Block = 0;
	//FSMC_NAND_WriteSpareArea_alt(dataToWrite, m_address, 1);

	FSMC_NAND_ReadSpareArea_alt(dataToRead, m_address, 1);

	//FSMC_NAND_WriteSmallPage_alt(dataToWrite, m_address, 1);

	FSMC_NAND_ReadSmallPage_alt(dataToRead, m_address, 1);



	m_address.Page = 5;

  // Infinite loop
  while (1)
    {
       // Add your code here.
    }
}

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
