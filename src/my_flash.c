#include "my_flash.h"
//temp
#include <stdio.h>
#include "diag/Trace.h"
//

/* Private define ------------------------------------------------------------*/
#define NAND_FLASH_START_ADDR     ((uint32_t)0x70000000)

void Flash_Test()
{
	NAND_ADDRESS m_WriteReadAddr;
	IDTypeDef  m_ID;
	uint8_t TxBuffer[NAND_PAGE_SIZE];
	uint16_t index;
	FSMC_Init();
	NAND_ReadID(&m_ID);
	if ((m_ID.Maker_ID == 0xEC) && (m_ID.Device_ID == 0xF1)
			&& (m_ID.Third_ID == 0x00) && (m_ID.Fourth_ID == 0x95))
	  {
		 printf("Type = K9F1G08U0B\r\n");
	  }

	m_WriteReadAddr.Zone = 0;
	m_WriteReadAddr.Block = 0;
	m_WriteReadAddr.Page = 0;

	uint32_t m_status;

	m_status = FSMC_NAND_EraseBlock(m_WriteReadAddr);
	if(m_status == NAND_READY)
	{
		printf("Erase_OK\r\n");
	}

	  /* Fill the buffer to send */
	  for (index = 0; index < NAND_PAGE_SIZE; index++ )
	  {
		 TxBuffer[index] = index;
	  }




}
void FSMC_Init()
{

	GPIO_init();
	NAND_init();


}

void NAND_init()
{
	  /*-- FSMC Configuration ------------------------------------------------------*/

	  __SYSCFG_CLK_ENABLE();
	  __FSMC_CLK_ENABLE();

	  FSMC_NAND_TypeDef *device;
	  device = FSMC_Bank2_3;



	  device -> PCR2 = 0x3E248;
	  device -> PMEM2 = 0xF1F2F3F1;
	  device -> PATT2 = 0xF1F2F3F1;

	  __FSMC_NAND_ENABLE(device, FSMC_NAND_BANK2);

	  return;
}

void NAND_ReadID(IDTypeDef* NAND_ID)
{
  uint32_t data = 0;

  /* Send Read ID command sequence */
    *(__IO uint8_t *)((uint32_t)(NAND_FLASH_START_ADDR | CMD_AREA))  = NAND_CMD_READID;
    *(__IO uint8_t *)((uint32_t)(NAND_FLASH_START_ADDR | ADDR_AREA)) = 0x00;

    /* Read the electronic signature from NAND flash */
    data = *(__IO uint32_t *)NAND_FLASH_START_ADDR;
  /* Send Command to the command area */
  /*(vu8 *)(NAND_FLASH_START_ADDR | CMD_AREA) = 0x90;
  *(vu8 *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x00; */

   /* Sequence to read ID from NAND flash
   data = *(vu32 *)(NAND_FLASH_START_ADDR | DATA_AREA);*/

   NAND_ID->Maker_ID   = ADDR_1st_CYCLE (data);
   NAND_ID->Device_ID  = ADDR_2nd_CYCLE (data);
   NAND_ID->Third_ID   = ADDR_3rd_CYCLE (data);
   NAND_ID->Fourth_ID  = ADDR_4th_CYCLE (data);
}

uint32_t FSMC_NAND_EraseBlock(NAND_ADDRESS Address)
{
	*(__IO uint8_t *)(uint32_t)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_ERASE0;

	*(__IO uint8_t *)(uint32_t)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	*(__IO uint8_t *)(uint32_t)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

	*(__IO uint8_t *)(uint32_t)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_ERASE1;


 // while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );

  return (FSMC_NAND_GetStatus());
}

uint32_t FSMC_NAND_GetStatus(void)
{
  uint32_t timeout = 0x1000000, status = NAND_READY;

  status = FSMC_NAND_ReadStatus();

  /* Wait for a NAND operation to complete or a TIMEOUT to occur */
  while ((status != NAND_READY) &&( timeout != 0x00))
  {
     status = FSMC_NAND_ReadStatus();
     timeout --;
  }

  if(timeout == 0x00)
  {
    status =  NAND_TIMEOUT_ERROR;
  }

  /* Return the operation status */
  return (status);
}

/******************************************************************************
* Function Name  : FSMC_NAND_ReadStatus
* Description    : Reads the NAND memory status using the Read status command
* Input          : None
* Output         : None
* Return         : The status of the NAND memory. This parameter can be:
*                   - NAND_BUSY: when memory is busy
*                   - NAND_READY: when memory is ready for the next operation
*                   - NAND_ERROR: when the previous operation gererates error
* Attention		 : None
*******************************************************************************/
uint32_t FSMC_NAND_ReadStatus(void)
{
  uint32_t data = 0x00, status = NAND_BUSY;

  /* Read status operation ------------------------------------ */
  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_STATUS;
  data = *(__IO uint8_t *)(NAND_FLASH_START_ADDR);

  if((data & NAND_ERROR) == NAND_ERROR)
  {
    status = NAND_ERROR;
  }
  else if((data & NAND_READY) == NAND_READY)
  {
    status = NAND_READY;
  }
  else
  {
    status = NAND_BUSY;
  }

  return (status);
}

void GPIO_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

		/** FSMC GPIO Configuration
		  PE7   ------> FSMC_D4+
		  PE8   ------> FSMC_D5+
		  PE9   ------> FSMC_D6+
		  PE10   ------> FSMC_D7+
		  PD11   ------> FSMC_CLE
		  PD12   ------> FSMC_ALE
		  PD14   ------> FSMC_D0+
		  PD15   ------> FSMC_D1+
		  PD0   ------> FSMC_D2+
		  PD1   ------> FSMC_D3+
		  PD4   ------> FSMC_NOE
		  PD5   ------> FSMC_NWE
		  PD6   ------> FSMC_NWAIT
		  PD7   ------> FSMC_NCE2 */

			// D4 -> D7
		__GPIOD_CLK_ENABLE();
		__GPIOE_CLK_ENABLE();

		GPIO_InitStructure.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStructure.Pull = GPIO_PULLUP;
		GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStructure.Alternate = GPIO_AF12_FSMC;
		 HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

		 /* D0 -> D4 PD0 */
		 GPIO_InitStructure.Pin = GPIO_PIN_14|GPIO_PIN_15
		                           |GPIO_PIN_0|GPIO_PIN_1;

		 GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		 GPIO_InitStructure.Pull = GPIO_PULLUP;
		 GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		 GPIO_InitStructure.Alternate = GPIO_AF12_FSMC;
		 HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

		 /* ALE CLE PD11 PD12*/
		 GPIO_InitStructure.Pin = GPIO_PIN_11|GPIO_PIN_12;
		 GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		 GPIO_InitStructure.Pull = GPIO_PULLUP;
		 GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		 GPIO_InitStructure.Alternate = GPIO_AF12_FSMC;
		 HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

		 /* NWE NOE PD5 PD4 */
		 GPIO_InitStructure.Pin = GPIO_PIN_4|GPIO_PIN_5;
		 GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		 GPIO_InitStructure.Pull = GPIO_PULLUP;
		 GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		 GPIO_InitStructure.Alternate = GPIO_AF12_FSMC;
		 HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

			/* R/B PD6 */
		 GPIO_InitStructure.Pin = GPIO_PIN_6;
		 GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		 GPIO_InitStructure.Pull = GPIO_PULLDOWN;
		 GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		 GPIO_InitStructure.Alternate = GPIO_AF12_FSMC;
		  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

			/* CE PD13 */
		  GPIO_InitStructure.Pin = GPIO_PIN_13;
		  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStructure.Pull = GPIO_PULLUP;
		  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		//  GPIO_InitStructure.Alternate = GPIO_AF12_FSMC;
		  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
		  /*-- GPIO Configuration ------------------------------------------------------*/
		  /* CLE, ALE, D0->D3, NOE, NWE and NCE2  NAND pin configuration  */

		  GPIOD -> MODER = 0xA6800A0A;
		  GPIOD -> OSPEEDR = 0xFFC00F0F;
		  GPIOD -> PUPDR = 0x55402505;
		  GPIOD -> IDR = 0x0000C7FF;

}

