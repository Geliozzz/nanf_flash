/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
  * @file    fsmc_nand_if.c 
  * @author  MCD Application Team
  * @version V3.0.0
  * @date    22-January-2013
  * @brief   manage NAND operations state machine

  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "fsmc_nand_if.h"
#include "nand_drv.h"


/* Private define ------------------------------------------------------------*/
#define NAND_FLASH_START_ADDR     ((uint32_t)0x70000000)
/** @addtogroup Libraries
  * @{
  */ 

/** @addtogroup NAND_Driver
  * @{
  */ 

/** @defgroup FSMC_NAND_IF_Private_TypesDefinitions
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup FSMC_NAND_IF_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup NAND_DRV_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup FSMC_NAND_IF_Private_Variables
  * @{
  */  
ONFI_NANDTypedef ONFI_NAND_STATE = NON_ONFI_NAND;
ONFI_NAND_ParamTypeDef ONFI_NandParam;
NumAddrCycleTypedef NUM_ADDR_CYCLE;    
NAND_TYPE_TypeDef NAND_TYPE = SBLK_NAND;
NAND_MAKER_TypeDef NAND_MAKER = ST_NAND;
uint32_t MassBlockSize;
uint32_t MassBlockCount;
uint8_t  MaxZone;
uint16_t MaxPhyBlockPerZone;
uint16_t MaxLogBlockPerZone;
uint16_t NandBlockSize;
uint16_t ActualSpareAreaSize;
uint16_t ActualPageSize;
uint16_t SblkSpareAreaSize;
uint16_t RW_PageSize;
uint8_t  Multiplier; 

/**
  * @}
  */ 
    
/** @defgroup FSMC_NAND_IF_Exported_Variables
  * @{
  */ 
 extern  COPY_STATE_NANDTypedef COPY_STATE;
 extern uint8_t Buff[]; 
/**
  * @}
  */ 
    
 

/** @defgroup FSMC_NAND_IF_Private_FunctionPrototypes
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup FSMC_NAND_IF_Private_Functions
  * @{
  */ 


/**
  * @brief  FSMC_SelectNANDType.
  *     This Function selects the NAND Type & 
  *     sets the Required Parameter accordingly.
  *     NAND may be SBLK_NAND or LBLK_NAND.
  * @param  None
  * @retval None
  */
void FSMC_SelectNANDType(void)
{
  ONFI_NAND_STATE = NON_ONFI_NAND;
  FSMC_NAND_ONFI_Compliance();

  if(ONFI_NAND_STATE == NON_ONFI_NAND)
  {
    FSMC_NAND_NON_ONFI_Compliance();
  }
  
}

/**
  * @brief  FSMC_NAND_NON_ONFI_Compliance.
  *     This Function selects the  NON ONFI NAND Type & 
  *     sets the Required Parameter accordingly.
  *     NAND may be SBLK_NAND or LBLK_NAND.
  * @param  None
  * @retval None
  */
void FSMC_NAND_NON_ONFI_Compliance(void)
{
  uint16_t deviceIndex = 0;
  uint8_t fourthID;
  uint16_t blkSize;
  uint8_t  arraySize;
  NAND_IDTypeDef nandID;
  FSMC_NAND_ReadID(&nandID);
  fourthID = nandID.Fourth_ID;
  
  /*Read the Number of Devices in Device Look Up Table*/
  arraySize = LBLK_NAND_END_INDEX + 1;  
  
  /*** Detect Manufacturer ***/
  switch(nandID.Maker_ID)
  {
    case(ST_MAKER_ID): 
      /**ST NAND**/
      NAND_MAKER = ST_NAND;
      break;
    case(SAMSUNG_MAKER_ID):
      /**SAMSUNG NAND**/
      NAND_MAKER = SAMSUNG_NAND;
      break;
    case(HYNIX_MAKER_ID):
      /**HYNIX NAND**/
      NAND_MAKER = HYNIX_NAND;
      break;
    case(TOSHIBA_MAKER_ID):
      /**TOSHIBA NAND**/
      NAND_MAKER = TOSHIBA_NAND;
      break;
    default:
      /**Other NAND**/    
    break;            
  }

  /*Detect the Particular Device from Device Look Up Table*/
    for(;deviceIndex<arraySize;deviceIndex++)
    {
      if(nandID.Device_ID == DeviceParameter[deviceIndex].DeviceCode)
      break;
    }

    SblkSpareAreaSize = SBLK_NAND_SPARE_AREA_SIZE;
    RW_PageSize = SBLK_NAND_PAGE_SIZE;
    MassBlockSize  = MASS_BLOCK_SIZE;
    MassBlockCount = DeviceParameter[deviceIndex].MSCBlockCount; 
    MaxZone  = DeviceParameter[deviceIndex].NumOfZone;
    NUM_ADDR_CYCLE = DeviceParameter[deviceIndex].NumAddrCycle;
  
  /**Followed from the DeviceParameter[] array(Device Look Up Table)**/
  if(deviceIndex <= SBLK_NAND_END_INDEX)
  {
    COPY_STATE = COPY_BACK_NOT_SUPPORTED;
    NAND_TYPE = SBLK_NAND;
    NandBlockSize = SBLK_NAND_BLOCK_SIZE;
  }
  
  /**Followed from the DeviceParameter[] array(Device Look Up Table)**/
        /****2K NAND****/
  else if((deviceIndex >= LBLK_NAND_START_INDEX)&&\
    (deviceIndex <= LBLK_NAND_END_INDEX))
  {
    NAND_TYPE = LBLK_NAND;

  }
  
  else
    while(1);   /**NAND FLASH not detected**/
 
  
     if(NAND_TYPE == LBLK_NAND)
     {
       switch(fourthID & BLOCK_SIZE_MASK)
        {
           case(BLOCK_SIZE_64K):
             blkSize = 64;
            break;
           case(BLOCK_SIZE_128K):
              blkSize = 128;
             break;
           case(BLOCK_SIZE_256K):
              blkSize = 256;
             break;
           case(BLOCK_SIZE_512K):
               blkSize  = 512;
              break;    
        }  
        
      
        switch(fourthID & PAGE_SIZE_MASK)
        {
           case(PAGE_SIZE_1K):
             
            break;
           case(PAGE_SIZE_2K):       //Page Size = 2048 + 64
                COPY_STATE = COPY_BACK_SUPPORTED;
                NandBlockSize = BLOCK_SIZE_2K_NAND;
                Multiplier = PAGE_MULTIPLE;     /* (2048/512B)*/
                ActualSpareAreaSize = SPARE_AREA_SIZE_2K_NAND;
                ActualPageSize = PAGE_SIZE_2K_NAND;
                
              break;
           case(PAGE_SIZE_4K):       //Page Size  = 4096 + 128
                COPY_STATE = COPY_BACK_NOT_SUPPORTED;
                Multiplier = 8;      // (4096/512B)
                NandBlockSize = (blkSize /4)*Multiplier;
                ActualSpareAreaSize = SblkSpareAreaSize*Multiplier;
                ActualPageSize = PAGE_SIZE_4K_NAND; 
                MaxZone = 1; 
                break;
           case(PAGE_SIZE_8K):     //Page Size  = 8192 + 256
                Multiplier = 16;      // (8192/512B)
                NandBlockSize = (blkSize /8)*Multiplier;
                ActualSpareAreaSize = SblkSpareAreaSize*Multiplier;
                ActualPageSize = 8192; 
                MaxZone = 1; 
              break;    
        } 
  }
  
  /**In NAND512R3B(Device ID == 0xA2) & NAND512W3B(Device ID == 0xF2)
       the max block per Zone => 512**/
  if((nandID.Device_ID == 0xA2)||(nandID.Device_ID == 0xF2))
  {
    MaxPhyBlockPerZone = MAX_PHY_BLOCKS_PER_ZONE/2;
    MaxLogBlockPerZone = MAX_LOG_BLOCKS_PER_ZONE/2;
  }
  else
  {
    MaxPhyBlockPerZone = MAX_PHY_BLOCKS_PER_ZONE;
    MaxLogBlockPerZone = MAX_LOG_BLOCKS_PER_ZONE;
  }
  
}

/**
  * @brief  FSMC_NAND_Init.
  *  Configures the FSMC and GPIOs to interface with the NAND memory.
  *               This function must be called before any write/read operation.
  * @param  None
  * @retval None
  */
void FSMC_Start(void)
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

	/*********************** GPIO ***************************************************/
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
 return;
}

/**
  * @brief  Reads NAND memory's ID.
  * @param  NAND_ID: pointer to a NAND_IDTypeDef structure which will hold
  *                    the Manufacturer and Device ID.
  * @retval None
  */

void FSMC_NAND_ReadID(NAND_IDTypeDef* NAND_ID)
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

/**
  * @brief  FSMC_NAND_WriteSmallPage.
  *      This routine is for writing one or several 512 Bytes Page size.
  * @param  pBuffer: pointer on the Buffer containing data to be written.
  * @param  Address: First page address.
  * @param  NumPageToWrite: Number of page to write.
  * @retval New status of the NAND operation. This parameter can be:
  *            NAND_TIMEOUT_ERROR: when the previous operation generate 
  *               a Timeout error.
  *            NAND_READY: when memory is ready for the next operation 
  *                        And the new status of the increment
  *                               address operation. It can be:
  *            NAND_VALID_ADDRESS: When the new address is valid address.
  *            NAND_INVALID_ADDRESS: When the new address is invalid address.
  */

uint32_t FSMC_NAND_WriteSmallPage(
                                  uint8_t *pBuffer,
                                  NAND_ADDRESS Address,
                                  uint32_t NumPageToWrite)
{
  uint32_t index = 0x00, numPageWritten = 0x00; 
  uint32_t addressStatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;
  uint32_t translateAddr = LBLK_ROW_ADDRESS(Address);
  uint32_t rowAddr,colAddr;
  switch(NAND_TYPE)
  {
    case SBLK_NAND:
     while((NumPageToWrite != 0x00) && 
           (addressStatus == NAND_VALID_ADDRESS) &&
           (status == NAND_READY))
        {             
        /* Page write command and address */
       COMMAND_REGISTER = SBLK_NAND_CMD_AREA_A;
       COMMAND_REGISTER = SBLK_NAND_CMD_WRITE0;
        
       FSMC_SBLK_NAND_SendAddress(Address);
         
        /* Calculate the size */
        size = SBLK_NAND_PAGE_SIZE *(1 + numPageWritten);

        /* Write data */
        for(; index < size; index++)
        {
          DATA_REGISTER = pBuffer[index];
        }
        
       COMMAND_REGISTER = SBLK_NAND_CMD_WRITE_TRUE1;

        /* Check status for successful operation */
        status = FSMC_NAND_GetStatus();
        
        if(status == NAND_READY)
        {
          numPageWritten++;

          NumPageToWrite--;

          /* Calculate Next small page Address */
          addressStatus = FSMC_NAND_AddressIncrement(&Address);    
        }    
      }
    return (status | addressStatus); 
    
  case LBLK_NAND:
     colAddr  = (translateAddr%Multiplier)*RW_PageSize;
     rowAddr  =  translateAddr/Multiplier;
      
      while((NumPageToWrite != 0x00) && 
            (addressStatus == NAND_VALID_ADDRESS) &&
            (status == NAND_READY))
      {
        /* Page write command and address */
       COMMAND_REGISTER = LBLK_NAND_CMD_PAGE_PROGRAM_CYCLE1;
         
        FSMC_LBLK_NAND_SendAddress(rowAddr,colAddr);
        
        /* Calculate the size */
        size = SBLK_NAND_PAGE_SIZE*(1 + numPageWritten);
        
        /** Write data **/
        for(; index < size; index++)
        {
          DATA_REGISTER = pBuffer[index];
        }
                 
       COMMAND_REGISTER = LBLK_NAND_CMD_PAGE_PROGRAM_CYCLE2;

        /* Check status for successful operation */
        status = FSMC_NAND_GetStatus();
        
        if(status == NAND_READY)
        {
          numPageWritten++;
          NumPageToWrite--;

          colAddr += RW_PageSize;          
          if(colAddr >= ActualPageSize)        
          {
            colAddr = 0;
            rowAddr += 1;
          }
      
        if(rowAddr > MaxPhyBlockPerZone*NandBlockSize )
             addressStatus = NAND_INVALID_ADDRESS; 
        }    
      }
    return (status | addressStatus); 
  default :
       status =   NAND_ERROR;
       return(status);   
  } 

}

/**
  * @brief  FSMC_NAND_ReadSmallPage.
  *       This routine is for sequential read from one or several 
  *        512 Bytes Page size.
  * @param  pBuffer: pointer on the Buffer to fill.
  * @param  Address: First page address.
  * @param  NumPageToRead: Number of page to read.
  * @retval New status of the NAND operation. This parameter can be:
  *          NAND_TIMEOUT_ERROR: when the previous operation generate 
  *              a Timeout error.
  *          NAND_READY: when memory is ready for the next operation. 
  *            And the new status of the increment address operation. It can be:
  *          NAND_VALID_ADDRESS: When the new address is valid address.
  *          NAND_INVALID_ADDRESS: When the new address is invalid address.
  */

uint32_t FSMC_NAND_ReadSmallPage(uint8_t *pBuffer,
                                 NAND_ADDRESS Address, 
                                 uint32_t NumPageToRead)
{
  uint32_t index = 0x00, numPageRead = 0x00, addressStatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;
  uint32_t translateAddr = LBLK_ROW_ADDRESS(Address);
  uint32_t rowAddr,colAddr;
  switch(NAND_TYPE)
  {
    case SBLK_NAND:
      while((NumPageToRead != 0x0) &&
            (addressStatus == NAND_VALID_ADDRESS))
      {	   
        /* Page Read Command and Page Address */
       COMMAND_REGISTER  = SBLK_NAND_CMD_AREA_A; 
       
        FSMC_SBLK_NAND_SendAddress(Address);
        
       COMMAND_REGISTER  = SBLK_NAND_CMD_AREA_TRUE1; 
 
        /* Calculate the Size */
        size = SBLK_NAND_PAGE_SIZE *(1 + numPageRead);
        
        /* Get Data into Buffer */    
        for(; index < size; index++)
        {
          pBuffer[index]= DATA_REGISTER;
        }

        numPageRead++;
        NumPageToRead--;

        /* Calculate page address */           			 
        addressStatus = FSMC_NAND_AddressIncrement(&Address);
      }

      status = FSMC_NAND_GetStatus();
      
      return (status | addressStatus);
      break;
  case LBLK_NAND:
        colAddr  = (translateAddr%Multiplier)*RW_PageSize;
        rowAddr  =  translateAddr/Multiplier;
            
      while((NumPageToRead != 0x0) && (addressStatus == NAND_VALID_ADDRESS))
      {	   
         /** Page Read Command **/
        COMMAND_REGISTER = LBLK_NAND_CMD_READ_CYCLE1; 

        /**Page Read Address**/
       FSMC_LBLK_NAND_SendAddress(rowAddr,colAddr);
        
        COMMAND_REGISTER = LBLK_NAND_CMD_READ_CYCLE2; 
         
        /* Calculate the Size */
        size = SBLK_NAND_PAGE_SIZE *(1 + numPageRead);
        
        
        /** Get Useful Data into Buffer **/    
        for(; index < size; index++)          
        {
          pBuffer[index]= DATA_REGISTER;
        }
        
        numPageRead++;
        NumPageToRead--;
         
        colAddr += RW_PageSize;           
        if(colAddr >= ActualPageSize)        
          {
           colAddr = 0;
           rowAddr += 1;
          }

        if(rowAddr > MaxPhyBlockPerZone*NandBlockSize )
               addressStatus = NAND_INVALID_ADDRESS;
        
      }

       status = FSMC_NAND_GetStatus();
      
      return (status | addressStatus);
  default :
       status =   NAND_ERROR;
       return(status);     
  }

  
}

/**
  * @brief  FSMC_NAND_WriteSpareArea.
  *      This routine write the spare area information for the specified 
  *               pages addresses.
  * @param  pBuffer: pointer on the Buffer containing data to be written.
  * @param  Address: First page address.
  * @param  NumSpareAreaTowrite: Number of Spare Area to write.
  * @retval New status of the NAND operation. This parameter can be:
  *      NAND_TIMEOUT_ERROR: when the previous operation generate 
  *           a Timeout error.
  *      NAND_READY: when memory is ready for the next operation 
  *         And the new status of the increment address operation. It can be:
  *      NAND_VALID_ADDRESS: When the new address is valid address.
  *      NAND_INVALID_ADDRESS: When the new address is invalid address.
  */
uint32_t FSMC_NAND_WriteSpareArea(uint8_t *pBuffer,
                                  NAND_ADDRESS Address,
                                  uint32_t NumSpareAreaTowrite)
{
	uint32_t index = 0x00, numSpareAreaWritten = 0x00;
	  uint32_t addressStatus = NAND_VALID_ADDRESS;
	  uint32_t status = NAND_READY, size = 0x00;
	  uint32_t translateAddr = LBLK_ROW_ADDRESS(Address);
	  uint32_t rowAddr,colAddr;

	  colAddr  = ActualPageSize + SblkSpareAreaSize*(translateAddr%Multiplier);
	        rowAddr  =  translateAddr/Multiplier;

	        while((NumSpareAreaTowrite != 0x00) &&
	              (addressStatus == NAND_VALID_ADDRESS) &&
	              (status == NAND_READY))
	        {
	          /* Page write command and address */
	        	/* Page write Spare area command and address */
	        	    *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_PAGEPROGRAM;

	        	    *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x00;
	        	    *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x08;
	        	    *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	        	    *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

	          /* Calculate the size */
	           size = SBLK_NAND_SPARE_AREA_SIZE *(1 + numSpareAreaWritten);

	          /** Write data **/
	          for(; index < size; index++)
	          {
	        	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | DATA_AREA) = pBuffer[index];
	          }

	          *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_PAGEPROGRAM_TRUE;

	       /* Check status for successful operation */
	        status = FSMC_NAND_GetStatus();

	        if(status == NAND_READY)
	        {
	          numSpareAreaWritten++;

	          NumSpareAreaTowrite--;

	          colAddr += SblkSpareAreaSize;

	         if(colAddr >=(ActualPageSize + ActualSpareAreaSize - SblkSpareAreaSize))
	            rowAddr += 1;
	        }
	      }
	      return (status | addressStatus);
}

/**
  * @brief  FSMC_NAND_ReadSpareArea.
  *    This routine read the spare area information from the specified
  *     pages addresses.
  * @param  pBuffer: pointer on the Buffer to fill.
  * @param  Address: First page address.
  * @param  NumSpareAreaToRead: Number of Spare Area to read.
  * @retval New status of the NAND operation. This parameter can be:
  *        NAND_TIMEOUT_ERROR: when the previous operation generate 
  *            a Timeout error.
  *        NAND_READY: when memory is ready for the next operation 
  *            And the new status of the increment address operation. It can be:
  *        NAND_VALID_ADDRESS: When the new address is valid address.
  *        NAND_INVALID_ADDRESS: When the new address is invalid address.
  */
uint32_t FSMC_NAND_ReadSpareArea(uint8_t *pBuffer,
                                 NAND_ADDRESS Address,
                                 uint32_t NumSpareAreaToRead)
{
	uint32_t numSpareAreaRead = 0x00, index = 0x00;
	  uint32_t addressStatus = NAND_VALID_ADDRESS;
	  uint32_t status = NAND_READY, size = 0x00;
	  uint32_t translateAddr = LBLK_ROW_ADDRESS(Address);
	  uint32_t rowAddr,colAddr;

	 // puts("Read Spare Area");

	    colAddr  = ActualPageSize + SblkSpareAreaSize*(translateAddr%Multiplier);
	    rowAddr  =  translateAddr/Multiplier;

	    while((NumSpareAreaToRead != 0x0) &&
	          (addressStatus == NAND_VALID_ADDRESS))
	    {
	      /** Page Read Command **/
	        /* Page Read command and page address */
	        *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_READ_1;

	        *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x00;
	        *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x08;
	        *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	        *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

	        *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_READ_TRUE;

	        /* ¶ÁÃ¦½Å */
	       // while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );
	        while(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == 0) {

	        } ;

	        for(int d = 0; d < 10000; d++)
	        {

	        }

	      /* Get Data into Buffer */
	      for ( ;index < size; index++)
	      {
	        pBuffer[index] = *(__IO uint8_t *)(NAND_FLASH_START_ADDR | DATA_AREA);
	      }
	      numSpareAreaRead++;

	      NumSpareAreaToRead--;

	      colAddr += SblkSpareAreaSize;

	      if(colAddr >=(ActualPageSize + ActualSpareAreaSize - SblkSpareAreaSize))
	        rowAddr += 1;

	    }

	    status = FSMC_NAND_GetStatus();

	    return (status | addressStatus);
  
}

/**
  * @brief  FSMC_NAND_EraseBlock.
  *    This routine erase complete block from NAND FLASH.
  * @param  Address: Any address into block to be erased.
  * @retval New status of the NAND operation. This parameter can be:
  *        NAND_TIMEOUT_ERROR: when the previous operation generate 
  *              a Timeout error.
  *        NAND_READY: when memory is ready for the next operation. 
  */
uint32_t FSMC_NAND_EraseBlock(NAND_ADDRESS Address)
{
  uint32_t status;
  uint32_t translateAddr = LBLK_ROW_ADDRESS(Address);  
  uint32_t rowAddr = translateAddr/Multiplier;
  uint16_t lblk = 0;
  uint16_t sblk = 0;
  switch(NAND_TYPE)
  {
    case SBLK_NAND:
          sblk =  (SBLK_ROW_ADDRESS(Address))/(SBLK_NAND_BLOCK_SIZE);
          COMMAND_REGISTER = SBLK_NAND_CMD_ERASE0;
        
          switch(NUM_ADDR_CYCLE)
          {
            case(THREE_CYCLE):
                ADDR_REGISTER = SBLK_CYCLE1(sblk);  
                ADDR_REGISTER = SBLK_CYCLE2(sblk); 
              break;
              
             case(FOUR_CYCLE):
                ADDR_REGISTER = SBLK_CYCLE1(sblk);  
                ADDR_REGISTER = SBLK_CYCLE2(sblk);  
                ADDR_REGISTER = SBLK_CYCLE3(sblk);  
              break;
          }
          
          COMMAND_REGISTER = SBLK_NAND_CMD_ERASE1; 

           return (FSMC_NAND_GetStatus());
       break;
    case LBLK_NAND:

          lblk =  rowAddr*Multiplier/NandBlockSize; 
          
          COMMAND_REGISTER = LBLK_NAND_CMD_BLOCK_ERASE_CYCLE1;
          
          switch(NUM_ADDR_CYCLE)
          {
            case(FOUR_CYCLE):
               ADDR_REGISTER = LBLK_CYCLE1(lblk);
               ADDR_REGISTER = LBLK_CYCLE2(lblk);
              break;
              
             case(FIVE_CYCLE):
                ADDR_REGISTER = LBLK_CYCLE1(lblk);
                ADDR_REGISTER = LBLK_CYCLE2(lblk);
                ADDR_REGISTER = LBLK_CYCLE3(lblk);
              break;
          }
 
          COMMAND_REGISTER = LBLK_NAND_CMD_BLOCK_ERASE_CYCLE2;

       return (FSMC_NAND_GetStatus());
  default :
       status =   NAND_ERROR;
       return(status);    
  }
}

/**
  * @brief  FSMC_NAND_Reset.
  *    This routine reset the NAND FLASH.
  * @param  None
  * @retval NAND_READY.
  */

uint32_t FSMC_NAND_Reset(void)
{
  uint32_t status;
  switch(NAND_TYPE)
  {
    case SBLK_NAND:
       COMMAND_REGISTER = SBLK_NAND_CMD_RESET;
         return (NAND_READY);
    case LBLK_NAND:
      COMMAND_REGISTER = LBLK_NAND_CMD_RESET;
      return(NAND_READY);
    default :
      status =   NAND_ERROR;
       return(status);  
  }
}

/**
  * @brief  FSMC_NAND_GetStatus.
  *          Get the NAND operation status.
  * @param  None
  * @retval New status of the NAND operation. This parameter can be:
  *        NAND_TIMEOUT_ERROR: when the previous operation generate 
  *            a Timeout error.
  *        NAND_READY: when memory is ready for the next operation. 
  */
uint32_t FSMC_NAND_GetStatus(void)
{
  uint32_t timeout = 0xFFFFFFFF, status = NAND_READY;

  switch(NAND_TYPE)
  {
  case SBLK_NAND:
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
  case LBLK_NAND:
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
  default:
      status =   NAND_ERROR;
      return(status);  
          
  }
  
}
/**
  * @brief  FSMC_SBLK_NAND_CopyBack.
  *   Copies One Page from Source Address to Destination Address
  *   without utilizing external Memory.
  * @param  src: Source Address.
  * @param  dest:Destination Address.
  * @retval The status of the NAND memory. This parameter can be:
  *           NAND_BUSY: when memory is busy.
  *           NAND_READY: when memory is ready for the next operation.    
  *           NAND_ERROR: when the previous operation gererates error. 
  */

uint32_t FSMC_SBLK_NAND_CopyBack(NAND_ADDRESS src,NAND_ADDRESS dest)
{
  uint32_t status  = NAND_READY; 
  
  /*  Send Source Address  */
  COMMAND_REGISTER  = SBLK_NAND_CMD_COPY_BACK_CYCLE1; 
  FSMC_SBLK_NAND_SendAddress(src);
  
  /* Send Destination Address*/
  COMMAND_REGISTER  = SBLK_NAND_CMD_COPY_BACK_CYCLE2; 
  
  FSMC_SBLK_NAND_SendAddress(dest);
  
  COMMAND_REGISTER  = SBLK_NAND_CMD_COPY_BACK_CYCLE3; 
  
  status = FSMC_NAND_GetStatus();
  
  return(status);

}

/**
  * @brief  FSMC_LBLK_NAND_CopyBack.
  *  Copies One Page from Source Address to Destination Address
  *  without utilizing external Memory.
  * @param  src: Source Address.
  * @param  dest:Destination Address.
  * @retval The status of the NAND memory. This parameter can be:
  *           NAND_BUSY: when memory is busy.
  *           NAND_READY: when memory is ready for the next operation.   
  *           NAND_ERROR: when the previous operation gererates error. 
  */

uint32_t FSMC_LBLK_NAND_CopyBack(NAND_ADDRESS src,NAND_ADDRESS dest)
{
  uint32_t status  = NAND_READY; 
  uint32_t translateAddr1 = LBLK_ROW_ADDRESS(src); 
  uint32_t translateAddr2 = LBLK_ROW_ADDRESS(dest);
  uint32_t rowAddr1,rowAddr2,colAddr;
  
  colAddr   = 0;
  rowAddr1  =  translateAddr1/Multiplier;
  rowAddr2  =  translateAddr2/Multiplier;
  
  /**Send Source Address**/
  COMMAND_REGISTER  = LBLK_NAND_CMD_COPY_BACK_CYCLE1;

  FSMC_LBLK_NAND_SendAddress(rowAddr1,colAddr);
  
  COMMAND_REGISTER  = LBLK_NAND_CMD_COPY_BACK_CYCLE2;

  /**Send Destination Address**/
  COMMAND_REGISTER  = LBLK_NAND_CMD_COPY_BACK_CYCLE3;
  
  FSMC_LBLK_NAND_SendAddress(rowAddr2,colAddr);
  
  COMMAND_REGISTER  = LBLK_NAND_CMD_COPY_BACK_CYCLE4;
  
  status = FSMC_NAND_GetStatus();
  return(status);
  
}
           
/**
  * @brief  FSMC_NAND_ReadStatus.
  *    Reads the NAND memory status using the Read status command.
  * @param  None
  * @retval The status of the NAND memory. This parameter can be:
  *            NAND_BUSY: when memory is busy.
  *            NAND_READY: when memory is ready for the next operation.    
  *            NAND_ERROR: when the previous operation gererates Error.
  */

uint32_t FSMC_NAND_ReadStatus(void)
{
  uint32_t data = 0x00, status = NAND_BUSY;

  switch(NAND_TYPE)
  {
  case SBLK_NAND:
          /* Read status operation ------------------- */
         COMMAND_REGISTER = SBLK_NAND_CMD_STATUS;
          data = DATA_REGISTER;

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
  case LBLK_NAND:
          /* Read status operation ----------------- */
         COMMAND_REGISTER = SBLK_NAND_CMD_STATUS;
          data = DATA_REGISTER;

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
  default:
      status =   NAND_ERROR;
       return(status);        
  }
}

/**
  * @brief  FSMC_NAND_AddressIncrement.
  *     Increment the NAND memory address.
  * @param  Address: address to be incremented.
  * @retval The new status of the increment address operation. It can be:
  *            NAND_VALID_ADDRESS: When the new address is valid address.
  *            NAND_INVALID_ADDRESS: When the new address is invalid address.
  */
uint32_t FSMC_NAND_AddressIncrement(NAND_ADDRESS* Address)
{
  uint32_t status = NAND_VALID_ADDRESS;
 
  switch(NAND_TYPE)
  {
  case SBLK_NAND:
          Address->Page++;
          if(Address->Page == SBLK_NAND_BLOCK_SIZE)
          {
            Address->Page = 0;
            Address->Block++;
            
            if(Address->Block == MaxLogBlockPerZone)
            {
              Address->Block = 0;
              Address->Zone++;

              if(Address->Zone == MaxZone)
              {
                status = NAND_INVALID_ADDRESS;
              }
            }
          } 
          return (status);
            
  case LBLK_NAND:
          Address->Page++;
          if(Address->Page == NandBlockSize)
          {
            Address->Page = 0;
            Address->Block++;
            
            if(Address->Block == MaxLogBlockPerZone)
            {
              Address->Block = 0;
              Address->Zone++;

              if(Address->Zone == MaxZone)
              {
                status = NAND_INVALID_ADDRESS;
              }
            }
          } 
          return (status);
   
  default:
      status =   NAND_ERROR;
      return(status);   
  }
}

/**
  * @brief  FSMC_NAND_ONFI_Compliance.
  *     This Function selects the ONFI NAND Type & 
  *     sets the Required Parameter accordingly.
  *     NAND may be SBLK_NAND or LBLK_NAND.
  * @param  None
  * @retval None
  */
void FSMC_NAND_ONFI_Compliance(void)
{
  uint16_t count;
  uint8_t index = 0;
  uint8_t sign[4];
  uint32_t temp; 
  /* Send Command to the Command Area */ 	
  COMMAND_REGISTER = CMD_READ_ELECTRONIC_SIGNATURE;
  ADDR_REGISTER = ONFI_SIGNATURE ;              
  
   for(count=0;count<4;count++)
   {
     sign[count] = DATA_REGISTER ;
   }
 
   /************************ONFI Compliance******************************/
 if((sign[0] == 'O')&&(sign[1] == 'N')&&(sign[2] == 'F')&&(sign[3] == 'I')) 
 {
   ONFI_NAND_STATE = ONFI_NAND;
   
   COMMAND_REGISTER = ONFI_PAGE_PARAM_READ_CMD;
   ADDR_REGISTER = 0x00 ;  
  
  for(count = 0;count<256;count++)
  { 
    Buff[count] = DATA_REGISTER; 
  }
  /******Followed from ONFI Specification Doc*********/
  for(index = 0;index<4;index++)
      ONFI_NandParam.PageSignature[index] = Buff[index];

  for(index = 0;index<12;index++)
      ONFI_NandParam.DeviceManufacturer[index] = Buff[index + 32];
  
  for(index = 0;index<20;index++)
     ONFI_NandParam.DeviceModel[index] = Buff[index + 44];
    
   ONFI_NandParam.DataBytePerPage = (Buff[80])|
                                    (Buff[81]<<8)|
                                    (Buff[82]<<16)|
                                    (Buff[83]<<24);
   
   ONFI_NandParam.SpareBytePerPage = (Buff[84])|
                                     (Buff[85]<<8);
   
   ONFI_NandParam.NumberOfPagePerBlock = (Buff[92])|
                                         (Buff[93]<<8)|
                                         (Buff[94]<<16)|
                                         (Buff[95]<<24);
   
   ONFI_NandParam.NumberOfBlockPerUnit = (Buff[96])|
                                         (Buff[97]<<8)|
                                         (Buff[98]<<16)|
                                         (Buff[99]<<24);
   ONFI_NandParam.NumberOfLogicalUnit =  Buff[100];
   ONFI_NandParam.NumberOfAddrCycle =    Buff[101];
   ONFI_NandParam.NumberOfBitsPerCell =  Buff[102];
   
   
   if(ONFI_NandParam.NumberOfAddrCycle == 0x22)
     NUM_ADDR_CYCLE = FOUR_CYCLE;
     
   if(ONFI_NandParam.NumberOfAddrCycle == 0x23)
     NUM_ADDR_CYCLE = FIVE_CYCLE;
   
   if(Buff[6]& 0x10)
    COPY_STATE = COPY_BACK_SUPPORTED;
   else
    COPY_STATE = COPY_BACK_NOT_SUPPORTED;
   
    NAND_TYPE = LBLK_NAND;
    Multiplier = (ONFI_NandParam.DataBytePerPage/512);   /* (PageSize/512B)*/         
    
    NandBlockSize = Multiplier *(ONFI_NandParam.NumberOfPagePerBlock );
    ActualSpareAreaSize = ONFI_NandParam.SpareBytePerPage; 
    ActualPageSize = ONFI_NandParam.DataBytePerPage;            

    SblkSpareAreaSize = SBLK_NAND_SPARE_AREA_SIZE;
    RW_PageSize = SBLK_NAND_PAGE_SIZE;
    MassBlockSize  = MASS_BLOCK_SIZE;

    temp = (uint32_t)(ONFI_NandParam.NumberOfLogicalUnit)*\
                     (ONFI_NandParam.NumberOfBlockPerUnit)*\
                     (ONFI_NandParam.NumberOfPagePerBlock)*\
                     (ONFI_NandParam.DataBytePerPage)/(MassBlockSize); 
    
    MassBlockCount = temp*MAX_LOG_BLOCKS_PER_ZONE/MAX_PHY_BLOCKS_PER_ZONE;
      
    MaxZone  = (ONFI_NandParam.NumberOfBlockPerUnit)*\
                 (ONFI_NandParam.NumberOfLogicalUnit)/MAX_PHY_BLOCKS_PER_ZONE;

    MaxPhyBlockPerZone = MAX_PHY_BLOCKS_PER_ZONE;
    MaxLogBlockPerZone = MAX_LOG_BLOCKS_PER_ZONE;

 }
}

/**
  * @brief  FSMC_SBLK_NAND_SendAddress.
  *     Sends the address for Small Block NAND.
  * @param  Addr: NAND_ADRESS to be sent.
  * @retval None.
  */
void FSMC_SBLK_NAND_SendAddress(NAND_ADDRESS Addr)
{
  
  switch(NUM_ADDR_CYCLE)
  {
    case(THREE_CYCLE):
        ADDR_REGISTER = 0x00;  
        ADDR_REGISTER = ADDR_CYCLE1(SBLK_ROW_ADDRESS(Addr));  
        ADDR_REGISTER = ADDR_CYCLE2(SBLK_ROW_ADDRESS(Addr));   
      break;
    case(FOUR_CYCLE):
        ADDR_REGISTER = 0x00;  
        ADDR_REGISTER = ADDR_CYCLE1(SBLK_ROW_ADDRESS(Addr));  
        ADDR_REGISTER = ADDR_CYCLE2(SBLK_ROW_ADDRESS(Addr));  
        ADDR_REGISTER = ADDR_CYCLE3(SBLK_ROW_ADDRESS(Addr)); 
      break; 
  }
}

/**
  * @brief  FSMC_LBLK_NAND_SendAddress.
  *     Sends the row & column address for Large Block NAND.
  * @param  row: Row Address.
  * @param  column: Columnn address.
  * @retval None.
  */

void FSMC_LBLK_NAND_SendAddress(uint32_t row,uint32_t column)
{
  switch(NUM_ADDR_CYCLE)
  {
      case(FOUR_CYCLE):
          ADDR_REGISTER = ADDR_CYCLE1(column);  
          ADDR_REGISTER = ADDR_CYCLE2(column);  
          ADDR_REGISTER = ADDR_CYCLE1(row);
          ADDR_REGISTER = ADDR_CYCLE2(row); 
       break;
      
      case(FIVE_CYCLE):
          ADDR_REGISTER = ADDR_CYCLE1(column);  
          ADDR_REGISTER = ADDR_CYCLE2(column);  
          ADDR_REGISTER = ADDR_CYCLE1(row);
          ADDR_REGISTER = ADDR_CYCLE2(row); 
          ADDR_REGISTER = ADDR_CYCLE3(row);
       break;
  }
}

/**
  * @}
  */ 
  
/**
  * @}
  */ 

/**
  * @}
  */ 

/******************************************************************************
* Function Name  : FSMC_NAND_WriteSpareArea
* Description    : This routine write the spare area information for the specified
*                  pages addresses.
* Input          : - pBuffer: pointer on the Buffer containing data to be written
*                  - Address: First page address
*                  - NumSpareAreaTowrite: Number of Spare Area to write
* Output         : None
* Return         : New status of the NAND operation. This parameter can be:
*                   - NAND_TIMEOUT_ERROR: when the previous operation generate
*                     a Timeout error
*                   - NAND_READY: when memory is ready for the next operation
*                  And the new status of the increment address operation. It can be:
*                  - NAND_VALID_ADDRESS: When the new address is valid address
*                  - NAND_INVALID_ADDRESS: When the new address is invalid address
* Attention		 : None
*******************************************************************************/
uint32_t FSMC_NAND_WriteSpareArea_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumSpareAreaTowrite)
{
  uint32_t index = 0x00, numsparesreawritten = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  puts("Write Spare Area");

  while((NumSpareAreaTowrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
  {
    /* Page write Spare area command and address */
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_PAGEPROGRAM;

	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x00;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x08;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

    /* Calculate the size */
    size = NAND_SPARE_AREA_SIZE + (NAND_SPARE_AREA_SIZE * numsparesreawritten);

    /* Write the data */
    for(; index < size; index++)
    {
    	*(__IO uint8_t *)(NAND_FLASH_START_ADDR | DATA_AREA) = pBuffer[index];
    }

    *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_PAGEPROGRAM_TRUE;

    /* ¶ÁÃ¦½Å */
    while(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == 0);

    /* Check status for successful operation */
    status = FSMC_NAND_GetStatus();

    if(status == NAND_READY)
    {
      numsparesreawritten++;

      NumSpareAreaTowrite--;

      /* Calculate Next page Address */
      addressstatus = FSMC_NAND_AddressIncrement(&Address);
    }
  }

  return (status | addressstatus);
}

/******************************************************************************
* Function Name  : FSMC_NAND_ReadSpareArea
* Description    : This routine read the spare area information from the specified
*                  pages addresses.
* Input          : - pBuffer: pointer on the Buffer to fill
*                  - Address: First page address
*                  - NumSpareAreaToRead: Number of Spare Area to read
* Output         : None
* Return         : New status of the NAND operation. This parameter can be:
*                   - NAND_TIMEOUT_ERROR: when the previous operation generate
*                     a Timeout error
*                   - NAND_READY: when memory is ready for the next operation
*                  And the new status of the increment address operation. It can be:
*                  - NAND_VALID_ADDRESS: When the new address is valid address
*                  - NAND_INVALID_ADDRESS: When the new address is invalid address
* Attention		 : None
*******************************************************************************/
uint32_t FSMC_NAND_ReadSpareArea_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumSpareAreaToRead)
{
  uint32_t numsparearearead = 0x00, index = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  puts("Read Spare Area Alt");

  while((NumSpareAreaToRead != 0x0) && (addressstatus == NAND_VALID_ADDRESS))
  {
    /* Page Read command and page address */
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_READ_1;

	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x00;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x08;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_READ_TRUE;

    /* ¶ÁÃ¦½Å */
   // while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );
    while(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == 0) {

    } ;

    for(int d = 0; d < 10000; d++)
    {

    }

    /* Data Read */
    size = NAND_SPARE_AREA_SIZE +  (NAND_SPARE_AREA_SIZE * numsparearearead);

    /* Get Data into Buffer */
    for ( ;index < size; index++)
    {
      pBuffer[index] =*(__IO uint8_t *)(NAND_FLASH_START_ADDR | DATA_AREA);
    }

    numsparearearead++;

    NumSpareAreaToRead--;

    /* Calculate page address */
    addressstatus = FSMC_NAND_AddressIncrement(&Address);
  }

  status = FSMC_NAND_GetStatus();

  return (status | addressstatus);
}

uint32_t FSMC_NAND_WriteSmallPage_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumPageToWrite)
{
  uint32_t index = 0x00, numpagewritten = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  while((NumPageToWrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
  {
    /* Page write command and address */
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_PAGEPROGRAM;

	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x00;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0X00;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

    /* Calculate the size */
    size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * numpagewritten);

    /* Write data */
    for(index=0; index < size; index++)
    {
    	*(__IO uint8_t *)(NAND_FLASH_START_ADDR | DATA_AREA) = pBuffer[index];
    }

    *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_PAGEPROGRAM_TRUE;

    /* ¶ÁÃ¦½Å */
//    while( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == 0 );

    /* Check status for successful operation */
    status = FSMC_NAND_GetStatus();

    if(status == NAND_READY)
    {
      numpagewritten++;

      NumPageToWrite--;

      /* Calculate Next small page Address */
      addressstatus = FSMC_NAND_AddressIncrement(&Address);
    }
  }

  return (status | addressstatus);
}

//*************************************************************************
uint32_t FSMC_NAND_ReadSmallPage_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumPageToRead)
{
  uint32_t index = 0x00, numpageread = 0x00, addressstatus = NAND_VALID_ADDRESS;
  uint32_t status = NAND_READY, size = 0x00;

  while((NumPageToRead != 0x0) && (addressstatus == NAND_VALID_ADDRESS))
  {
    /* Page Read command and page address */
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_READ_1;

	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0x00;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = 0X00;
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

	  *(__IO uint8_t *)(NAND_FLASH_START_ADDR | CMD_AREA) = NAND_CMD_READ_TRUE;

    /* ¶ÁÃ¦½Å */
    //while( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == 0 );

    /* Calculate the size */
    size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * numpageread);

	for(index=0; index < 0x255; index++);//ÑÓÊ±

    /* Get Data into Buffer */
    for(index=0; index < size; index++)
    {
      pBuffer[index]= *(__IO uint8_t *)(NAND_FLASH_START_ADDR | DATA_AREA);
    }

    numpageread++;

    NumPageToRead--;

    /* Calculate page address */
    addressstatus = FSMC_NAND_AddressIncrement(&Address);
  }

  status = FSMC_NAND_GetStatus();

  return (status | addressstatus);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
