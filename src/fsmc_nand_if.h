/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
  * @file    fsmc_nand_if.h 
  * @author  MCD Application Team
  * @version V3.0.0
  * @date    22-January-2013
  * @brief   Header for fsmc_nand_if.c file
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
#ifndef __FSMC_NAND_IF_H
#define __FSMC_NAND_IF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
//#include "stm32f4xx_ll_fsmc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"

/** @addtogroup Libraries
  * @{
  */ 

/** @addtogroup NAND_Driver
  * @{
  */ 

#define USE_PCB_NAND 5;

/** @defgroup FSMC_NAND_IF_Exported_Macros
  * @{
  */ 

#if defined(USE_SOCKET_NAND)
    #define FSMC_BANK_NAND FSMC_Bank3_NAND
    #define Bank_NAND_ADDR ((uint32_t)0x80000000)
#endif 

#if defined(USE_PCB_NAND) 
  #define FSMC_BANK_NAND FSMC_NAND_BANK2
  #define Bank_NAND_ADDR   ((uint32_t)0x70000000)
#endif

/**ROW ADDRESS for SMALL BLOCK NAND**/
#define SBLK_ROW_ADDRESS(Address) (uint32_t)(Address.Page + \
(Address.Block + (Address.Zone * MaxPhyBlockPerZone)) * NandBlockSize)

/**ROW ADDRESS for LARGE BLOCK NAND**/
#define LBLK_ROW_ADDRESS(Address) (Address.Page + (Address.Block +\
    (Address.Zone * MaxPhyBlockPerZone)) * NandBlockSize)

#define COL_ADDRESS         ((uint8_t)(0x00))

/** NAND Area definition **/
#define CMD_AREA                   (uint32_t)(1<<17)  /* A17 = CLE  high */
#define ADDR_AREA                  (uint32_t)(1<<16)  /* A16 = ALE high */
#define DATA_AREA                  ((uint32_t)0x00000000) 

/*********************SMALL BLOCK NAND Memory Command**************************/
#define	SBLK_NAND_CMD_AREA_A            ((uint8_t)0x00)
#define	SBLK_NAND_CMD_AREA_B            ((uint8_t)0x01)
#define SBLK_NAND_CMD_AREA_C            ((uint8_t)0x50)
#define SBLK_NAND_CMD_AREA_TRUE1        ((uint8_t)0x30)

#define SBLK_NAND_CMD_WRITE0            ((uint8_t)0x80)
#define SBLK_NAND_CMD_WRITE_TRUE1       ((uint8_t)0x10)
	
#define SBLK_NAND_CMD_ERASE0            ((uint8_t)0x60)
#define SBLK_NAND_CMD_ERASE1            ((uint8_t)0xD0)  

#define SBLK_NAND_CMD_READ_ID           ((uint8_t)0x90)	
#define SBLK_NAND_CMD_STATUS            ((uint8_t)0x70)
#define SBLK_NAND_CMD_LOCK_STATUS       ((uint8_t)0x7A)
#define SBLK_NAND_CMD_RESET             ((uint8_t)0xFF)

#define SBLK_NAND_CMD_COPY_BACK_CYCLE1  ((uint8_t)0x00)
#define SBLK_NAND_CMD_COPY_BACK_CYCLE2  ((uint8_t)0x8A)
#define SBLK_NAND_CMD_COPY_BACK_CYCLE3  ((uint8_t)0x10)


/*****************************************************************************/



/*********************LARGE BLOCK NAND Memory Command************************/

    /****Read Command***/
#define LBLK_NAND_CMD_READ_CYCLE1              ((uint8_t)0x00)
#define LBLK_NAND_CMD_READ_CYCLE2              ((uint8_t)0x30)

    /**Random Data Output**/
#define LBLK_NAND_CMD_RANDOM_DATA_OUT_CYCLE1   ((uint8_t)0x05)
#define LBLK_NAND_CMD_RANDOM_DATA_OUT_CYCLE2   ((uint8_t)0xE0)

    /***Cache Read***/
#define LBLK_NAND_CMD_CACHE_READ_CYCLE1        ((uint8_t)0x00)
#define LBLK_NAND_CMD_CACHE_READ_CYCLE2        ((uint8_t)0x31)

    /***Exit Cache Read***/
#define LBLK_NAND_CMD_EXIT_CACHE_READ          ((uint8_t)0x34)

    /****Page Program***/
#define LBLK_NAND_CMD_PAGE_PROGRAM_CYCLE1      ((uint8_t)0x80)
#define LBLK_NAND_CMD_PAGE_PROGRAM_CYCLE2      ((uint8_t)0x10)

    /**Random Data Input**/
#define LBLK_NAND_CMD_RANDOM_DATA_IN           ((uint8_t)0x85)

    /***Copy Back Program***/
#define LBLK_NAND_CMD_COPY_BACK_CYCLE1         ((uint8_t)0x00)
#define LBLK_NAND_CMD_COPY_BACK_CYCLE2         ((uint8_t)0x35)
#define LBLK_NAND_CMD_COPY_BACK_CYCLE3         ((uint8_t)0x85)
#define LBLK_NAND_CMD_COPY_BACK_CYCLE4         ((uint8_t)0x10)

    /**Cache Program**/
#define LBLK_NAND_CMD_CACHE_PROGRAM_CYCLE1     ((uint8_t)0x80)
#define LBLK_NAND_CMD_CACHE_PROGRAM_CYCLE2     ((uint8_t)0x15)

    /**Block Erase**/
#define LBLK_NAND_CMD_BLOCK_ERASE_CYCLE1       ((uint8_t)0x60)
#define LBLK_NAND_CMD_BLOCK_ERASE_CYCLE2       ((uint8_t)0xD0)

    /**Reset**/
#define LBLK_NAND_CMD_RESET                    ((uint8_t)0xFF)

    /**Read Electronic Signature**/
#define CMD_READ_ELECTRONIC_SIGNATURE          ((uint8_t)0x90)

    /**Read Status Register**/
#define LBLK_NAND_CMD_READ_STATUS_REGISTER     ((uint8_t)0x70)

    /**Read Block Lock Status**/
#define LBLK_NAND_CMD_READ_BLOCK_LOCK_STATUS   ((uint8_t)0x7A)

    /**Block Unlock**/
#define LBLK_NAND_CMD_BLOCK_UNLOCK_CYCLE1      ((uint8_t)0x23)
#define LBLK_NAND_CMD_BLOCK_UNLOCK_CYCLE2      ((uint8_t)0x24)

    /**Block Lock**/
#define LBLK_NAND_CMD_BLOCK_LOCK               ((uint8_t)0x2A)

    /**Block Lock Down**/
#define LBLK_NAND_CMD_BLOCK_LOCK_DOWN          ((uint8_t)0x2C)

/* FSMC NAND memory address computation */
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFF)               /* 1st addressing cycle */
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00) >> 8)      /* 2nd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000) >> 16)   /* 3rd addressing cycle */
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000) >> 24) /* 4th addressing cycle */

/*****************************************************************************/


/*********** NAND memory status ************/
#define NAND_VALID_ADDRESS         ((uint32_t)0x00000100)
#define NAND_INVALID_ADDRESS       ((uint32_t)0x00000200)
#define NAND_TIMEOUT_ERROR         ((uint32_t)0x00000400)
#define NAND_BUSY                  ((uint32_t)0x00000000)
#define NAND_ERROR                 ((uint32_t)0x00000001)
#define NAND_READY                 ((uint32_t)0x00000040)

/************************ FSMC NAND memory parameters ***********************/
      /****************SMALL BLOCK  NAND***********/
/* 512 Bytes per page w/o Spare Area */
#define SBLK_NAND_PAGE_SIZE             ((uint16_t)0x0200) 
/* 32x512 Bytes pages per block */
#define SBLK_NAND_BLOCK_SIZE            ((uint16_t)0x0020) 
/* 1024 Block per zone */
#define SBLK_NAND_ZONE_SIZE             ((uint16_t)0x0400) 
/* Last 16 Bytes as spare area */
#define SBLK_NAND_SPARE_AREA_SIZE       ((uint16_t)0x0010) 
/* 4 Zones of 1024 Block */
#define SBLK_NAND_MAX_ZONE              ((uint16_t)0x0004) 


/****************************LARGE BLOCK NAND******************************/
/**************************************************************************/

 /******************2K NAND****************************/
/* 2048 Bytes per page w/o Spare Area */
#define PAGE_SIZE_2K_NAND             ((uint16_t)0x0800) 
/* 64x2048 Bytes pages per Block */
/*****Number of pages in Multiple of 512B***/
#define BLOCK_SIZE_2K_NAND            ((uint16_t)0x0100)   /* 64x4 */
/* 1024 Block per Zone */
#define ZONE_SIZE_2K_NAND             ((uint16_t)0x0400) 
/* Last 64 Bytes as Spare Area */
#define SPARE_AREA_SIZE_2K_NAND       ((uint16_t)0x0040) 
/* 1 Zones of 512 Block */
#define MAX_ZONE_2K_NAND              ((uint16_t)0x0001) 

/*********************4K NAND******************/
/* 4096 Bytes per page w/o Spare Area */
#define PAGE_SIZE_4K_NAND             ((uint16_t)0x1000) 
/* 64x2048 Bytes pages per Block */
/*****Number of pages in Multiple of 512B***/
#define BLOCK_SIZE_4K_NAND            ((uint16_t)0x0200)   /* 64x8 */
/* 1024 Block per Zone */
#define ZONE_SIZE_4K_NAND             ((uint16_t)0x0400) 
/* Last 128 Bytes as Spare Area */
#define SPARE_AREA_SIZE_4K_NAND       ((uint16_t)0x0080) 
/* 1 Zones of 512 Block */
#define MAX_ZONE_4K_NAND              ((uint16_t)0x0001) 


/************************************************************************/
/************************************************************************/

/*************FSMC NAND memory Address Computation ********************/
 /* 1st addressing cycle */
#define ADDR_CYCLE1(ADDR)       (uint8_t)((ADDR)& 0x000000FF)  
 /* 2nd addressing cycle */
#define ADDR_CYCLE2(ADDR)       (uint8_t)(((ADDR)& 0x0000FF00) >> 8)
/* 3rd addressing cycle */
#define ADDR_CYCLE3(ADDR)       (uint8_t)(((ADDR)& 0x00FF0000) >> 16)   
/* 4th addressing cycle */
#define ADDR_CYCLE4(ADDR)       (uint8_t)(((ADDR)& 0xFF000000) >> 24) 
/*********************************************************************/

#define SBLK_CYCLE1(sblk)  (uint8_t)((sblk &0x0007)<<5)
#define SBLK_CYCLE2(sblk)  (uint8_t)((sblk &0x07F8)>>3)
#define SBLK_CYCLE3(sblk)  (uint8_t)((sblk &0x1800)>>11)

#define LBLK_CYCLE1(lblk)  (uint8_t)((lblk &0x0003)<<6)
#define LBLK_CYCLE2(lblk)  (uint8_t)((lblk &0x03FC)>>2)
#define LBLK_CYCLE3(lblk)  (uint8_t)((lblk &0x1C00)>>10)

#define COMMAND_REGISTER  *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define ADDR_REGISTER     *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define DATA_REGISTER     *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)

#define ST_MAKER_ID                0x20
#define SAMSUNG_MAKER_ID           0xEC
#define HYNIX_MAKER_ID             0xAD
#define TOSHIBA_MAKER_ID           0x98

#define SBLK_NAND_START_INDEX      0
#define SBLK_NAND_END_INDEX        7
#define LBLK_NAND_START_INDEX      8
#define LBLK_NAND_END_INDEX        17

#define MASS_BLOCK_SIZE            512



#define PAGE_SIZE_MASK   0x03
#define PAGE_SIZE_1K     0x00
#define PAGE_SIZE_2K     0x01
#define PAGE_SIZE_4K     0x02
#define PAGE_SIZE_8K     0x03


#define BLOCK_SIZE_MASK     0x30
#define BLOCK_SIZE_64K      0x00
#define BLOCK_SIZE_128K     0x10
#define BLOCK_SIZE_256K     0x20
#define BLOCK_SIZE_512K     0x30




#define ONFI_SIGNATURE                ((uint8_t)0x20)  
#define ONFI_PAGE_PARAM_READ_CMD      ((uint8_t)0xEC) 


/**
  * @}
  */ 

/** @defgroup FSMC_NAND_IF_Expoted_TypesDefinitions
  * @{
  */ 
typedef enum
{
  ONFI_NAND,
  NON_ONFI_NAND
}ONFI_NANDTypedef;

typedef enum
{
  THREE_CYCLE,
  FOUR_CYCLE,
  FIVE_CYCLE
}NumAddrCycleTypedef;

typedef struct 
{
  NumAddrCycleTypedef NumAddrCycle;
  uint8_t  NumOfZone;
  uint8_t  DeviceCode;
  uint32_t MSCBlockCount;
}DeviceParameterTypeDef;

typedef struct
{
  uint8_t Maker_ID;
  uint8_t Device_ID;
  uint8_t Third_ID;
  uint8_t Fourth_ID;
}NAND_IDTypeDef;

typedef struct
{
  uint8_t  PageSignature[4];
  uint8_t  DeviceManufacturer[12];
  uint8_t  DeviceModel[20];
  uint32_t DataBytePerPage;
  uint16_t SpareBytePerPage;
  uint32_t NumberOfPagePerBlock;
  uint32_t NumberOfBlockPerUnit;
  uint8_t  NumberOfLogicalUnit;
  uint8_t  NumberOfAddrCycle;
  uint8_t  NumberOfBitsPerCell;
}ONFI_NAND_ParamTypeDef;

typedef struct 
{
  uint16_t Zone;
  uint16_t Block;
  uint16_t Page;
} NAND_ADDRESS;

typedef enum
{
  SBLK_NAND,          //Page size (512B + 16B)      SMALL BLOCK NAND
  LBLK_NAND,          //Page size (2048B + 64B)     LARGE BLOCK NAND
}NAND_TYPE_TypeDef;

typedef enum
{
 ST_NAND,
 TOSHIBA_NAND,
 SAMSUNG_NAND,
 HYNIX_NAND 
}NAND_MAKER_TypeDef;

/**
  * @}
  */ 

/** @defgroup FSMC_NAND_IF_Expoted_Variables
  * @{
  */ 

/*****An Array of structure containing Device Code & Corresponding
          Mass Block Count 
(Mass Block Count = MAX_ZONE*MAX_LOG_BLOCK*NAND_BLOCK_SIZE*NAND_PAGE_SIZE/512*/



static DeviceParameterTypeDef DeviceParameter[LBLK_NAND_END_INDEX+1] =
{
  /*************************Small Block NAND*********************************/
  /*************************x8 Device, Page size(512B +16B)*****************/
  
  {THREE_CYCLE,1,0x33, 32000},         //NAND128R3A      Array index   0
  {THREE_CYCLE,1,0x73, 32000},         //NAND128W3A                    1
  {THREE_CYCLE,2,0x35, 64000},         //NAND256R3A                    2
  {THREE_CYCLE,2,0x75, 64000},         //NAND256W3A                    3
  {FOUR_CYCLE, 4,0x36,128000},         //NAND512R3A                    4
  {FOUR_CYCLE, 4,0x76,128000},         //NAND512W3A                    5
  {FOUR_CYCLE, 8,0x39,256000},         //NAND01GR3A                    6
  {FOUR_CYCLE, 8,0x79,256000},         //NAND01GW3A                    7
                
 
/***************************Large Block NAND********************************/
/*************************x8 Device, Page size (2048B+64B)*****************/
                   
  {FOUR_CYCLE,1,0xA2, 128000},           //NAND512R3B                  8 
  {FOUR_CYCLE,1,0xF2, 128000},           //NAND512W3B                  9
  {FOUR_CYCLE,1,0xA1, 256000},           //NAND01GR3B                  10
  {FOUR_CYCLE,1,0xF1, 256000},           //NAND01GW3B                  11
  {FIVE_CYCLE,2,0xAA, 512000},           //NAND02GR3B                  12
  {FIVE_CYCLE,2,0xDA, 512000},           //NAND02GW3B                  13
  {FIVE_CYCLE,4,0xAC, 1024000},          //NAND04GR3B                  14
  {FIVE_CYCLE,4,0xDC, 1024000},          //NAND04GW3B                  15
  {FIVE_CYCLE,8,0xA3, 2048000},          //NAND08GR3B                  16
  {FIVE_CYCLE,8,0xD3, 2048000},          //NAND08GW3B                  17
};

/**
  * @}
  */ 
//TESSSSSSSSSSSSSSSSSSSSSSSSSSSST

/* FSMC NAND memory command */
#define	NAND_CMD_READ_1             ((uint8_t)0x00)
#define	NAND_CMD_READ_TRUE          ((uint8_t)0x30)

#define	NAND_CMD_RDCOPYBACK         ((uint8_t)0x00)
#define	NAND_CMD_RDCOPYBACK_TRUE    ((uint8_t)0x35)

#define NAND_CMD_PAGEPROGRAM        ((uint8_t)0x80)
#define NAND_CMD_PAGEPROGRAM_TRUE   ((uint8_t)0x10)

#define NAND_CMD_COPYBACKPGM        ((uint8_t)0x85)
#define NAND_CMD_COPYBACKPGM_TRUE   ((uint8_t)0x10)

#define NAND_CMD_ERASE0             ((uint8_t)0x60)
#define NAND_CMD_ERASE1             ((uint8_t)0xD0)

#define NAND_CMD_READID             ((uint8_t)0x90)
#define NAND_CMD_STATUS             ((uint8_t)0x70)
#define NAND_CMD_RESET              ((uint8_t)0xFF)

#define NAND_CMD_CACHEPGM           ((uint8_t)0x80)
#define NAND_CMD_CACHEPGM_TRUE      ((uint8_t)0x15)

#define NAND_CMD_RANDOMIN           ((uint8_t)0x85)
#define NAND_CMD_RANDOMOUT          ((uint8_t)0x05)
#define NAND_CMD_RANDOMOUT_TRUE     ((uint8_t)0xE0)

#define NAND_CMD_CACHERD_START      ((uint8_t)0x00)
#define NAND_CMD_CACHERD_START2     ((uint8_t)0x31)
#define NAND_CMD_CACHERD_EXIT       ((uint8_t)0x34)

/* for K9F1G08 */
#define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2 * 1024 bytes per page w/o Spare Area */
#define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64 pages per block */
#define NAND_ZONE_SIZE             ((uint16_t)0x0400) /* 1024 Block per zone */
#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 1024 block */

#define ROW_ADDRESS (Address.Page + (Address.Block + (Address.Zone * NAND_ZONE_SIZE)) * NAND_BLOCK_SIZE)


//TESSSSSSSSSSSSSSSSSSSSSSSSSSST

/** @defgroup FSMC_NAND_IF_Exported_FunctionPrototypes
  * @{
  */ 
void FSMC_Start(void);
void FSMC_SelectNANDType(void);
void FSMC_NAND_ReadID(NAND_IDTypeDef* NAND_ID);
void FSMC_NAND_ONFI_Compliance(void);
void FSMC_NAND_NON_ONFI_Compliance(void);
uint32_t FSMC_NAND_Reset(void);
uint32_t FSMC_NAND_GetStatus(void);
uint32_t FSMC_NAND_ReadStatus(void);
void FSMC_SBLK_NAND_SendAddress(NAND_ADDRESS Addr);
void FSMC_LBLK_NAND_SendAddress(uint32_t row,uint32_t Column);
uint32_t FSMC_NAND_AddressIncrement(NAND_ADDRESS* Address);
uint32_t FSMC_SBLK_NAND_CopyBack(NAND_ADDRESS src,
                                 NAND_ADDRESS dest);
uint32_t FSMC_LBLK_NAND_CopyBack(NAND_ADDRESS src,
                                 NAND_ADDRESS dest);
uint32_t FSMC_NAND_WriteSmallPage(uint8_t *pBuffer,
                                  NAND_ADDRESS Address, 
                                  uint32_t NumPageToWrite);
uint32_t FSMC_NAND_ReadSmallPage (uint8_t *pBuffer,
                                  NAND_ADDRESS Address,
                                  uint32_t NumPageToRead);
uint32_t FSMC_NAND_WriteSpareArea(uint8_t *pBuffer,
                                  NAND_ADDRESS Address,
                                  uint32_t NumSpareAreaTowrite);
uint32_t FSMC_NAND_ReadSpareArea(uint8_t *pBuffer, 
                                 NAND_ADDRESS Address,
                                 uint32_t  NumSpareAreaToRead);
uint32_t FSMC_NAND_EraseBlock(NAND_ADDRESS Address);

// Alternative functions
uint32_t FSMC_NAND_ReadSpareArea_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumSpareAreaToRead);
uint32_t FSMC_NAND_WriteSpareArea_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumSpareAreaTowrite);
uint32_t FSMC_NAND_ReadSmallPage_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumPageToRead);
uint32_t FSMC_NAND_WriteSmallPage_alt(uint8_t *pBuffer, NAND_ADDRESS Address, uint32_t NumPageToWrite);
#endif /* __FSMC_NAND_IF_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
