/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
  * @file    nand_drv.h 
  * @author  MCD Application Team
  * @version V3.0.0
  * @date    22-January-2013
  * @brief   Header for nand_drv.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NAND_DRV_H
#define __NAND_DRV_H

/* Includes ------------------------------------------------------------------*/
#include "fsmc_nand_if.h"
#include "stm32f4xx.h"

/** @addtogroup Libraries
  * @{
  */ 
/* FSMC PCRx Mask */
#define PCR_PBKEN_SET          ((uint32_t)0x00000004)
#define PCR_PBKEN_RESET        ((uint32_t)0x000FFFFB)
#define PCR_ECCEN_SET          ((uint32_t)0x00000040)
#define PCR_ECCEN_RESET        ((uint32_t)0x000FFFBF)
#define PCR_MEMORYTYPE_NAND    ((uint32_t)0x00000008)

/** @addtogroup NAND_Driver
  * @{
  */ 

/** @defgroup NAND_DRV_Exported_Defines
  * @{
  */ 
#define WEAR_DEPTH                   10
#define TWO_ERRORS                   0xFF
#define PAGE_TO_WRITE(NumByte)       (NumByte/512)


#define NAND_OK   0
#define NAND_FAIL 1

#define FREE_BLOCK  (1 << 12 )
#define BAD_BLOCK   (1 << 13 )
#define VALID_BLOCK (1 << 14 )
#define USED_BLOCK  (1 << 15 )

#define MAX_PHY_BLOCKS_PER_ZONE  1024
#define MAX_LOG_BLOCKS_PER_ZONE  1000
#define MAX_FREE_BLOCKS_PER_ZONE  24

#define MASK_LUT           (0x0000FFFF)
#define MASK_WL            (0xFFFF0000)
    
#define PHY_BLOCK_MASK         (0x03FF)        /* 0x03FF == 1023 */
#define PAGE_MULTIPLE     4  /*PAGE SIZE in Multiple of 512 Byte(2048/512 = 4)*/

#define SBLK_PAGE_NUMBER(index,Zone) (uint32_t) (index * SBLK_NAND_BLOCK_SIZE+\
    (Zone * SBLK_NAND_BLOCK_SIZE * MaxPhyBlockPerZone))
 
#define LBLK_PAGE_NUMBER(index,Zone)  (uint32_t)((index*NandBlockSize +\
      (Zone * NandBlockSize * MaxPhyBlockPerZone)))



/**
  * @}
  */ 


/** @defgroup NAND_DRV_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup NAND_DRV_Exported_Variables
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup NAND_DRV_Exported_Types
  * @{
  */

typedef struct  
{
  uint16_t LogicalIndex;
  uint16_t WearLevel; 
  uint8_t  DataStatus;
  uint8_t  BlockStatus;
  uint8_t  reserved[6];
  uint32_t ECC;
} SPARE_AREA;		

typedef struct  
{
  uint8_t  BlockStatus;
  uint8_t  DataStatus;
  uint16_t LogicalIndex;
  uint16_t WearLevel; 
  uint8_t  reserved[6];
  uint32_t ECC;
} LBLK_SPARE_AREA;


typedef enum {
  WRITE_IDLE = 0,
  POST_WRITE,
  PRE_WRITE,
  WRITE_CLEANUP,
  WRITE_ONGOING  
}WRITE_STATE;  

typedef enum {
  OLD_BLOCK = 0,
  UNUSED_BLOCK
}BLOCK_STATE; 

typedef enum
{
  COPY_BACK_SUPPORTED,
  COPY_BACK_NOT_SUPPORTED
}COPY_STATE_NANDTypedef;


/**
  * @}
  */

/** @defgroup NAND_DRV_ExportedFunctionsPrototype
  * @{
  */
    
void SBLK_NAND_PostWrite (void);
void LBLK_NAND_PostWrite (void);
uint16_t SBLK_NAND_BuildLUT (uint8_t ZoneNum);
uint16_t LBLK_NAND_BuildLUT (uint8_t ZoneNum);
uint16_t NAND_UpdateWearLevelCounter(NAND_ADDRESS Address);
void SBLK_NAND_WritePage(NAND_ADDRESS Address , uint8_t *buff , uint16_t len);
void LBLK_NAND_WritePage(NAND_ADDRESS Address , uint8_t *buff , uint16_t len);
void SBLK_NAND_ReadPage (NAND_ADDRESS Address , uint8_t *buff , uint16_t len );
void LBLK_NAND_ReadPage (NAND_ADDRESS Address , uint8_t *buff , uint16_t len );
uint16_t SBLK_NAND_WearLeveling (uint8_t ZoneNumber);
uint16_t LBLK_NAND_WearLeveling (uint8_t ZoneNumber);
uint16_t NAND_GetFreeBlock (void);
uint16_t NAND_PostWrite(void);
uint16_t NAND_PostWriteECC(void);
uint16_t NAND_CopyBack(NAND_ADDRESS Address_Src,
                       NAND_ADDRESS Address_Dest,
                       uint16_t PageToCopy);
void NAND_WritePage (NAND_ADDRESS Address ,
                     uint8_t *buff ,
                     uint16_t len );
void NAND_ReadPage (NAND_ADDRESS Address ,
                    uint8_t *buff ,
                    uint16_t len );
uint16_t NAND_Copy(NAND_ADDRESS Address_Src,
                   NAND_ADDRESS Address_Dest,
                   uint16_t PageToCopy);
uint16_t NAND_Init (void);
uint16_t NAND_GarbageCollection(void);
uint16_t NAND_Format (void);
uint16_t Swap (uint16_t num);
uint8_t BitCount(uint32_t num);
NAND_ADDRESS NAND_ConvertPhyAddress (uint32_t Address);
uint8_t GetParity (uint16_t in_value);
uint16_t NAND_BuildLUT(uint8_t ZoneNbr);
uint16_t NAND_WearLeveling (uint8_t ZoneNbr);
uint16_t NAND_CleanLUT (uint8_t ZoneNbr);
LBLK_SPARE_AREA LBLK_NAND_ReadSpareArea (uint32_t address);
SPARE_AREA SBLK_NAND_ReadSpareArea(uint32_t address);
uint16_t WriteSpareArea (uint32_t address,
                         uint8_t *buff);
uint16_t NAND_Write (uint32_t Memory_Offset,
                     uint8_t *Writebuff,
                     uint16_t NumByte);
uint16_t NAND_Read  (uint32_t Memory_Offset,
                     uint8_t *Readbuff,
                     uint16_t NumByte);
uint16_t NAND_WriteECC(uint32_t Memory_Offset,
                        uint8_t *Writebuff,
                        uint16_t NumByte);
#endif
/**
  * @}
  */

/**
  * @}
  */ 
  
/**
  * @}
  */ 

void FSMC_NANDCmd(uint32_t FSMC_Bank, FunctionalState NewState);
void FSMC_NANDECCCmd(uint32_t FSMC_Bank, FunctionalState NewState);
uint32_t FSMC_GetECC(uint32_t FSMC_Bank);

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
