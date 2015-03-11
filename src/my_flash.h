#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_fsmc.h"
#include "stm32f4xx_hal_gpio.h"

/* FSMC NAND memory parameters */
/* for K9F1G08 */
#define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2 * 1024 bytes per page w/o Spare Area */
#define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64 pages per block */
#define NAND_ZONE_SIZE             ((uint16_t)0x0400) /* 1024 Block per zone */
#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 1024 block */

#define ROW_ADDRESS (Address.Page + (Address.Block + (Address.Zone * NAND_ZONE_SIZE)) * NAND_BLOCK_SIZE)


#define FSMC_Bank2_NAND                          ((uint32_t)0x00000010)

/** @defgroup FSMC_Wait_feature
  * @{
  */
#define FSMC_Waitfeature_Disable                 ((uint32_t)0x00000000)
#define FSMC_Waitfeature_Enable                  ((uint32_t)0x00000002)


/** @defgroup FSMC_ECC
  * @{
  */
#define FSMC_ECC_Disable                         ((uint32_t)0x00000000)
#define FSMC_ECC_Enable                          ((uint32_t)0x00000040)


/** @defgroup FSMC_ECC_Page_Size
  * @{
  */
#define FSMC_ECCPageSize_256Bytes                ((uint32_t)0x00000000)
#define FSMC_ECCPageSize_512Bytes                ((uint32_t)0x00020000)
#define FSMC_ECCPageSize_1024Bytes               ((uint32_t)0x00040000)
#define FSMC_ECCPageSize_2048Bytes               ((uint32_t)0x00060000)
#define FSMC_ECCPageSize_4096Bytes               ((uint32_t)0x00080000)
#define FSMC_ECCPageSize_8192Bytes               ((uint32_t)0x000A0000)


/** @defgroup FSMC_Data_Width
  * @{
  */

#define FSMC_MemoryDataWidth_8b                  ((uint32_t)0x00000000)
#define FSMC_MemoryDataWidth_16b                 ((uint32_t)0x00000010)
#define IS_FSMC_MEMORY_WIDTH(WIDTH) (((WIDTH) == FSMC_MemoryDataWidth_8b) || \
                                     ((WIDTH) == FSMC_MemoryDataWidth_16b))

/* FSMC NAND memory address computation */
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFF)               /* 1st addressing cycle */
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00) >> 8)      /* 2nd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000) >> 16)   /* 3rd addressing cycle */
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000) >> 24) /* 4th addressing cycle */

typedef struct
{
  uint8_t Maker_ID;
  uint8_t Device_ID;
  uint8_t Third_ID;
  uint8_t Fourth_ID;
}
IDTypeDef;

typedef struct
{
  uint16_t Zone;
  uint16_t Block;
  uint16_t Page;
}
NAND_ADDRESS;


void Flash_Test();
void FSMC_Init();
void NAND_ReadID(IDTypeDef* NAND_ID);
void GPIO_init();
void NAND_init();
uint32_t FSMC_NAND_GetStatus(void);
uint32_t FSMC_NAND_ReadStatus(void);
uint32_t FSMC_NAND_EraseBlock(NAND_ADDRESS Address);
