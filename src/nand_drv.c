/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
  * @file    nand_drv.c 
  * @author  MCD Application Team
  * @version V3.0.0
  * @date    22-January-2013
  * @brief   Manage NAND operations state machine
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
#include "nand_drv.h"
#include <string.h>
#include "stm32f4xx.h"

/** @addtogroup Libraries
  * @{
  */ 

/** @addtogroup NAND_Driver
  * @{
  */ 

/** @defgroup NAND_DRV_Private_TypesDefinitions
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


/** @defgroup NAND_DRV_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup NAND_DRV_Private_Variables
  * @{
  */ 
uint32_t LUT[1024] = {0};      /*Look Up Table Buffer*/
NAND_ADDRESS Victim[1024];     /*Table for Victim Block*/
uint16_t ArrayIndex[1024] = {0};
uint16_t WearCountArray[1024] = {0};/*Wear Count of the Block Pointed by LUT*/
uint8_t Buff[512];
WRITE_STATE Write_State;
BLOCK_STATE Block_State;
COPY_STATE_NANDTypedef COPY_STATE = COPY_BACK_NOT_SUPPORTED;
NAND_ADDRESS WAddress, FAddress;
uint16_t  PhyBlock;
uint16_t  LogAddress, InitialPage = 0, CurrentZone = 0;
uint16_t  WrittenPages = 0; 
uint16_t  VictimIdx=0,VictimBlocks=0;
uint16_t  OldIndex,NewIndex;
uint32_t  SCSI_BlkLength = 1;
uint8_t   ECC_CorrectableErrorFlag = 0;

/**
  * @}
  */ 

/** @defgroup NAND_DRV_Exported_Variables
  * @{
  */
extern uint16_t RW_PageSize;
extern NAND_TYPE_TypeDef NAND_TYPE;
extern uint8_t  Multiplier;
extern uint8_t  MaxZone;
extern uint16_t MaxPhyBlockPerZone;
extern uint16_t MaxLogBlockPerZone;
extern uint16_t NandBlockSize;
extern ONFI_NANDTypedef ONFI_NAND_STATE ;
/**
  * @}
  */ 

/** @defgroup NAND_DRV_Private_FunctionPrototypes
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup NAND_DRV_Private_Functions
  * @{
  */ 

/**
* @brief  NAND_Init.
*         Initialize NAND Interface.
* @param  None
* @retval Status of NAND Initialization. This Parameter can be 
*            NAND_OK: when the NAND is OK. 
*            NAND_FAIL: when NAND fails to initialize.
*/
uint16_t NAND_Init(void)
{
  uint16_t status = NAND_OK;
  FSMC_Start();
  FSMC_SelectNANDType();
  status = NAND_CleanLUT (0);
  Write_State = WRITE_IDLE;
  return status;
}

/**
* @brief  NAND_Write.
*         write one sector by once.
* @param  Memory_Offset: Memory Offset.
* @param  Writebuff: Pointer to the data to be written.
* @param  Transfer_Length: Number of byte to write.
* @retval Status of NAND Write. This Parameter can be 
*           NAND_OK: when the NAND Write is successful 
*           NAND_FAIL: when NAND fails to Write.
*/
uint16_t NAND_Write(uint32_t Memory_Offset,
                    uint8_t *Writebuff,
                    uint16_t Transfer_Length)
{ 
  uint16_t nandBlkSize;
  #ifdef FAT_FS
   Memory_Offset *= RW_PageSize;
  #endif
   
  nandBlkSize = NandBlockSize;
   
  /* Check Block status and calculate start and end addreses */
  WAddress = NAND_ConvertPhyAddress(Memory_Offset / RW_PageSize);
  
  
  /*Check Zone: if second zone is requested build second LUT*/
  if (WAddress.Zone != CurrentZone)
  {
    CurrentZone = WAddress.Zone;
    NAND_CleanLUT(CurrentZone); 
  }
  
  PhyBlock    = LUT[WAddress.Block]&MASK_LUT;  /* Block Index + flags */
  LogAddress  = WAddress.Block ;              /* save logical Block */
  
  OldIndex = WAddress.Block;                                

  if ( Write_State == WRITE_IDLE)  /* IDLE state */
  {
    if (PhyBlock & USED_BLOCK)  /* USED BLOCK */
    { 
      Block_State = OLD_BLOCK;
      
      /* Get a free Block for swap */
      FAddress.Block = NAND_GetFreeBlock();    /*Get Physical Block*/
      FAddress.Zone  = WAddress.Zone;

      InitialPage = FAddress.Page  = WAddress.Page;
      
      /* write the new page */
      NAND_WritePage (FAddress ,
                      (uint8_t *)Writebuff ,
                      PAGE_TO_WRITE(Transfer_Length));   
      WrittenPages++;
      
      /* Get physical Block of previous */
      WAddress.Block = PhyBlock & PHY_BLOCK_MASK;      /* 0x3FF = 1023*/
      
      /* SCSI_BlkLength-->7th and 8th Byte in write10 cmd 
      i.e total no. of pages to write */
      if (WrittenPages == SCSI_BlkLength)      
      {
        NAND_PostWrite();
        WrittenPages = 0;
        
        return NAND_OK;
      }
      else
      {
        if (WAddress.Page == (nandBlkSize - 1))
        {
          NAND_PostWrite();
          return NAND_OK;
        }
        Write_State = WRITE_ONGOING;
        return NAND_OK;
      }
    }
    else     /* UNUSED BLOCK */
    {
      Block_State = UNUSED_BLOCK;
      /* Write the new page */
      WAddress.Block = PhyBlock & PHY_BLOCK_MASK;
      NAND_WritePage (WAddress ,
                      (uint8_t *)Writebuff ,
                      PAGE_TO_WRITE(Transfer_Length)); 
      WrittenPages++;
      if (WrittenPages == SCSI_BlkLength)
      {
        WrittenPages = 0;
        NAND_PostWrite();
        Write_State = WRITE_IDLE;
        return NAND_OK;
      }
      else
      {
        Write_State = WRITE_ONGOING;
        return NAND_OK;
      }
    }
  }
  /* WRITE state */
  if ( Write_State == WRITE_ONGOING)   /* Write Ongoing state */
  {
    if (PhyBlock & USED_BLOCK)        /* USED BLOCK */
    {  
      WAddress.Block = PhyBlock & PHY_BLOCK_MASK;
      Block_State = OLD_BLOCK;
      FAddress.Page  = WAddress.Page;
      
      /* Check if next pages are in next Block */
      if (WAddress.Page == (nandBlkSize - 1))
      {
        /* write Last page  */
        NAND_WritePage (FAddress ,
                        (uint8_t *)Writebuff ,
                        PAGE_TO_WRITE(Transfer_Length));   
        WrittenPages++;
        if (WrittenPages == SCSI_BlkLength)
        {
          WrittenPages = 0;        
        }
        /* Clean up and Update the LUT */
        NAND_PostWrite();
        Write_State = WRITE_IDLE;
        return NAND_OK;
      }
      
      /* Write next page */
      NAND_WritePage (FAddress ,
                      (uint8_t *)Writebuff ,
                      PAGE_TO_WRITE(Transfer_Length));   
      WrittenPages++;      
      
      if (WrittenPages == SCSI_BlkLength)
      {
        NAND_PostWrite();
        Write_State = WRITE_IDLE;
        WrittenPages = 0;
      }
    }
    else    /* UNUSED BLOCK */
    {
      WAddress.Block = PhyBlock & PHY_BLOCK_MASK;
      /* Check if it is the last page in prev Block */
      if (WAddress.Page == (SBLK_NAND_BLOCK_SIZE - 1))
      {
        /* Write Last page  */
        NAND_WritePage (WAddress ,
                        (uint8_t *)Writebuff ,
                        PAGE_TO_WRITE(Transfer_Length));   
        WrittenPages++;
        if (WrittenPages == SCSI_BlkLength)
        {
          WrittenPages = 0;
        }
        /* Clean up and Update the LUT */
        NAND_PostWrite();
        Write_State = WRITE_IDLE;
        return NAND_OK;
      }
      /* Write next page in same Block */
      NAND_WritePage (WAddress ,
                      (uint8_t *)Writebuff ,
                      PAGE_TO_WRITE(Transfer_Length)); 
      WrittenPages++;
      if (WrittenPages == SCSI_BlkLength)
      {
        NAND_PostWrite();
        Write_State = WRITE_IDLE;
        WrittenPages = 0;
      }
    }
  }
  return NAND_OK;
}

/**
* @brief  NAND_Read.
*         Read sectors.
* @param  Memory_Offset: Memory Offset.
* @param  Readbuff: Pointer to store the read data.
* @param  Transfer_Length: Number of byte to read.
* @retval Status of NAND Read. This Parameter can be 
*            NAND_OK: when the NAND Read is successful.
*            NAND_FAIL: when NAND fails to Read.
*/
uint16_t NAND_Read(
                   uint32_t Memory_Offset,
                   uint8_t *Readbuff, 
                   uint16_t Transfer_Length)
{
  NAND_ADDRESS phAddress;
#ifdef FAT_FS
  Memory_Offset *= RW_PageSize;
#endif

  phAddress = NAND_ConvertPhyAddress(Memory_Offset / RW_PageSize);
  
  if (phAddress.Zone != CurrentZone)
  {
    CurrentZone = phAddress.Zone;
    NAND_BuildLUT(CurrentZone);
  }
  
  if ((LUT [phAddress.Block]&MASK_LUT) & BAD_BLOCK)
  {
    return NAND_FAIL;
  }
  else
  {
    phAddress.Block = (LUT [phAddress.Block]) & ~ (USED_BLOCK | VALID_BLOCK);
    
    /*Read a page using the Error Correction Code*/
    NAND_ReadPage (phAddress, (uint8_t *)Readbuff ,Transfer_Length/RW_PageSize);
    
    if(ECC_CorrectableErrorFlag)
    {
      NAND_WriteECC(Memory_Offset,Readbuff,Transfer_Length);
      ECC_CorrectableErrorFlag = 0;
    }
  }
  return NAND_OK;
}

/**
* @brief  NAND_WriteECC.
*         write one sector & copy rest Block during ECC Correctable Error Case.
* @param  Memory_Offset: Memory Offset.
* @param  Writebuff: Pointer to the data to be written.
* @param  Transfer_Length: Number of byte to write.
* @retval Status of NAND Write. This Parameter can be 
*           NAND_OK: when the NAND Write is successful 
*           NAND_FAIL: when NAND fails to Write.
*/

uint16_t  NAND_WriteECC(uint32_t Memory_Offset,
                         uint8_t *Writebuff,
                         uint16_t NumByte)
{
#ifdef FAT_FS
  Memory_Offset *= RW_PageSize;
#endif
  
  /* Check Block status and calculate start and end addreses */
  WAddress = NAND_ConvertPhyAddress(Memory_Offset / RW_PageSize);
  
  /*Check Zone: if second zone is requested build second LUT*/
  if (WAddress.Zone != CurrentZone)
  {
    CurrentZone = WAddress.Zone;
    NAND_CleanLUT(CurrentZone); 
  }
  
  PhyBlock    = LUT[WAddress.Block]&MASK_LUT;  /* Block Index + flags */
  LogAddress  = WAddress.Block ;              /* save logical Block */
  
  OldIndex = WAddress.Block;                                
  
  /* Get a free Block for swap */
  FAddress.Block = NAND_GetFreeBlock(); /*Get Physical Block*/
  FAddress.Zone  = WAddress.Zone;
  
  InitialPage = FAddress.Page  = WAddress.Page;
  
  /* write the new page */
  NAND_WritePage (FAddress ,(uint8_t *)Writebuff ,1);   
  
  /* Get physical Block of previous */
  WAddress.Block = PhyBlock & PHY_BLOCK_MASK;      /* 0x3FF = 1023*/
  
  NAND_PostWriteECC();
  
  return NAND_OK;
}

/**
* @brief  NAND_PostWriteECC.
*         Copies whole block after writing corrected page in ECC Correction.
* @param  Memory_Offset: Memory Offset.
* @param  Writebuff: Pointer to the data to be written.
* @param  Transfer_Length: Number of byte to write.
* @retval Status of NAND Write. This Parameter can be 
*           NAND_OK: when the NAND Write is successful 
*           NAND_FAIL: when NAND fails to Write.
*/
uint16_t NAND_PostWriteECC(void)
{
  uint32_t swapIdx;
  uint16_t  tempSpareArea [8];
  uint16_t  pageBack;
  uint32_t  swapVariable;
  NAND_ADDRESS tempNandAddress;
  
  /* Precopy Old first pages */
  if (InitialPage != 0)
  {
    pageBack = WAddress.Page;                     
    FAddress.Page = WAddress.Page = 0;
    
    FSMC_NAND_ReadSmallPage  ((uint8_t *)Buff, WAddress , 1 );
    FSMC_NAND_WriteSmallPage ((uint8_t *)Buff, FAddress, 1);
    
    FSMC_NAND_ReadSpareArea((uint8_t *)tempSpareArea, WAddress, 1);
    
    tempSpareArea [0] = 0xFFFF;
    tempSpareArea [1] = 0xFFFF;
    tempSpareArea [2] = 0xFFFF; 
    tempSpareArea [3] = 0xFFFF; 
    tempSpareArea [4] = 0xFFFF; 
    tempSpareArea [5] = 0xFFFF; 
    
    FSMC_NAND_WriteSpareArea((uint8_t *)tempSpareArea,FAddress, 1);          
    
    FSMC_NAND_AddressIncrement(&WAddress);
    FSMC_NAND_AddressIncrement(&FAddress);
    
    if(COPY_STATE == COPY_BACK_SUPPORTED)
      NAND_CopyBack(WAddress, FAddress, InitialPage -1 );
    else
      NAND_Copy(WAddress, FAddress, InitialPage -1 );
    WAddress.Page =  pageBack ;
    
  }
  
  /* Postcopy remaining pages */
  if ((NandBlockSize - (WAddress.Page + 1)) != 0)
  {
    FSMC_NAND_AddressIncrement(&WAddress);
    FAddress.Page = WAddress.Page;
      if(COPY_STATE == COPY_BACK_SUPPORTED)
        NAND_CopyBack(WAddress, FAddress, NandBlockSize - WAddress.Page);
      else
        NAND_Copy(WAddress, FAddress, NandBlockSize - WAddress.Page);   
  }
  
  if(NAND_TYPE == SBLK_NAND)
  {
    tempSpareArea [0] = Swap((LogAddress << 1)| (GetParity(LogAddress)));
    tempSpareArea [1] = 0xFFFF; 
  }
  
  if(NAND_TYPE == LBLK_NAND)
  { 
    tempSpareArea [0] = 0xFFFF;
    tempSpareArea [1] = Swap((LogAddress << 1)| (GetParity(LogAddress)));
  }
  
  tempSpareArea [2] = 0xFFFF; 
  tempSpareArea [3] = 0xFFFF; 
  tempSpareArea [4] = 0xFFFF; 
  tempSpareArea [5] = 0xFFFF; 
  tempSpareArea [6] = 0xFFFF; 
  tempSpareArea [7] = 0xFFFF;   
  
  FAddress.Page     = 0x00;
  
  FSMC_NAND_WriteSpareArea( (uint8_t *)tempSpareArea , FAddress , 1);
  
  swapIdx = LUT[OldIndex] & 0x03FF;
  swapVariable  = LUT[OldIndex];
  LUT[OldIndex] = LUT[NewIndex];
  LUT[NewIndex] = swapVariable;
  
  /*Read the SPARE AREA before Erasing the OLD Block*/
  tempNandAddress = WAddress;
  tempNandAddress.Page = 0;
  FSMC_NAND_ReadSpareArea( (uint8_t *)tempSpareArea , tempNandAddress , 1);
  
  WAddress.Page = 0;
  /*Erase the Block*/
  FSMC_NAND_EraseBlock(WAddress);
  
  if(NAND_TYPE == SBLK_NAND)
  {
    tempSpareArea [0] = 0xFFFF;
    tempSpareArea [1] += 2; 
    tempSpareArea [2] = 0xFFFF; 
  }
  
  if(NAND_TYPE == LBLK_NAND)
  {  
    tempSpareArea [0] = 0xFFFF;
    tempSpareArea [1] = 0xFFFF;
    tempSpareArea [2] += 2; 
  }

  tempSpareArea [3] = 0xFFFF; 
  tempSpareArea [4] = 0xFFFF; 
  tempSpareArea [5] = 0xFFFF; 
  tempSpareArea [6] = 0xFFFF; 
  tempSpareArea [7] = 0xFFFF; 
  
  /* Write the Spare Area on 0th page of the Erased Block*/
  FSMC_NAND_WriteSpareArea( (uint8_t *)tempSpareArea , tempNandAddress ,1);
  
  /**Update LUT**/
  LUT[NewIndex] = (uint32_t)(0xFFFF<<16)|swapIdx;
  LUT[OldIndex] ^= (USED_BLOCK |VALID_BLOCK);
  
  return NAND_OK;
}

/**
* @brief  NAND_CleanLUT.
*         Rebuild the Look Up Table.
* @param  ZoneNbr: Zone Number to Rebuild the Look Up Table.
* @retval Status of NAND Build look up table . This Parameter can be 
*            NAND_OK: when the NAND Clean is successful.
*            NAND_FAIL: when NAND fails to clean look up table.
*/
uint16_t NAND_CleanLUT (uint8_t ZoneNum)
{
  /* Build the LUT for the current zone */
  NAND_BuildLUT (ZoneNum);

  /*Apply the wear leveling for the current zone*/
  NAND_WearLeveling (ZoneNum);

  return NAND_OK;
}

/**
* @brief  NAND_WearLeveling.
*          Build the Look Up Table According to the Wear Count.
* @param  ZoneNumber: Zone Number.
* @retval Status of NAND wear Leveling . This Parameter can be 
*            NAND_OK: when the NAND wear leveling is successful 
*            NAND_FAIL: when NAND fails to wear leveling.
*/
uint16_t NAND_WearLeveling (uint8_t ZoneNumber)
{ 
    switch(NAND_TYPE)
    {
      case (SBLK_NAND):
        SBLK_NAND_WearLeveling(ZoneNumber);
        break;
      
      case(LBLK_NAND):
        LBLK_NAND_WearLeveling(ZoneNumber);
        break; 
    }
    return NAND_OK;
}

/**
* @brief  SBLK_NAND_WearLeveling.
*         Build the Look Up Table According to the Wear Count.
* @param  ZoneNumber: Zone Number.
* @retval Status of SBLK_NAND_WearLeveling . This Parameter can be 
*            NAND_OK: when the NAND wear leveling is successful 
*            NAND_FAIL: when NAND fails to wear leveling.
*/
uint16_t SBLK_NAND_WearLeveling (uint8_t ZoneNumber)
{                                  
  SPARE_AREA spareArea;
  uint16_t x;
  uint16_t pCurrentBlock;
  uint32_t swapSort;
  uint16_t idxWL = 0,loopVar1,loopVar2;
  uint16_t count  = 0;
  uint16_t temp;
  uint8_t cond1,cond2,cond3;
  
  /**Sets the array Element to Zero***/
  memset(ArrayIndex,0,sizeof(ArrayIndex));
  memset(WearCountArray,0,sizeof(WearCountArray));
  
  /*Sorting of Free Block in the ascending order of Wear leveling counter*/
  for ( pCurrentBlock = 0 ; pCurrentBlock <= MaxPhyBlockPerZone -1 ;\
    pCurrentBlock++)
  {
      if ((!((LUT[pCurrentBlock] & USED_BLOCK) == USED_BLOCK)) && \
        (!((LUT[pCurrentBlock] & BAD_BLOCK) == BAD_BLOCK)))
      {
         spareArea = SBLK_NAND_ReadSpareArea(SBLK_PAGE_NUMBER\
                ((LUT[pCurrentBlock]&PHY_BLOCK_MASK),ZoneNumber));

       /*Store Wear Count of all Free Blocks*/
       WearCountArray[idxWL] = spareArea.WearLevel;
       
       /*Store LUT index of free block*/
       ArrayIndex[idxWL] = pCurrentBlock;
       idxWL++;
       count++;  
      }
  }
 
  for ( loopVar1 = 0 ; loopVar1 < count-1; loopVar1++)
  {
    for ( loopVar2 = 0 ; loopVar2 < count-loopVar1-1 ; loopVar2++)
    {    
      cond1 =  (WearCountArray[loopVar2] != 0xFFFF);
      cond2 =  ((cond1)&& (WearCountArray[loopVar2+1] == 0xFFFF));
      cond3 = cond1&&(WearCountArray[loopVar2] > WearCountArray[loopVar2+1]);
      
      if(cond2 ||cond3)   /* 0xFFFF ->Unused Block */
      {                                              
       swapSort = LUT[ArrayIndex[loopVar2]];
       LUT[ArrayIndex[loopVar2]] = LUT[ArrayIndex[loopVar2+1]];
       LUT[ArrayIndex[loopVar2+1]] = swapSort;  

       temp = WearCountArray[loopVar2];
       WearCountArray[loopVar2] = WearCountArray[loopVar2+1];
       WearCountArray[loopVar2+1]  = temp; 
      }       
    }
  }     
 return NAND_OK;
}

/**
* @brief  LBLK_NAND_WearLeveling.
*         Build the Look Up Table According to the Wear Count.
* @param  ZoneNumber: Zone Number.
* @retval Status of LBLK_NAND_WearLeveling . This Parameter can be 
*            NAND_OK: when the NAND wear leveling is successful 
*            NAND_FAIL: when NAND fails to wear leveling.
*/
uint16_t LBLK_NAND_WearLeveling (uint8_t ZoneNumber)
{ 
  LBLK_SPARE_AREA spareArea;
  uint16_t pCurrentBlock = 0;
  uint32_t swapSort;
  uint16_t idxWL = 0,loopVar1,loopVar2;
  uint16_t count  = 0;
  uint16_t temp;
  uint8_t cond1,cond2,cond3;
  
  /**Sets the array Element to Zero***/
  memset(ArrayIndex,0,sizeof(ArrayIndex));
  memset(WearCountArray,0,sizeof(WearCountArray));
  
  /*Sorting of Free Block in the ascending order of Wear leveling counter*/
  for (; pCurrentBlock <= MaxPhyBlockPerZone -1 ;pCurrentBlock++)
  {

      if ((!((LUT[pCurrentBlock] & USED_BLOCK) == USED_BLOCK)) && \
        (!((LUT[pCurrentBlock] & BAD_BLOCK) == BAD_BLOCK)))
      {
        
      spareArea = LBLK_NAND_ReadSpareArea(LBLK_PAGE_NUMBER\
        ((LUT[pCurrentBlock]&PHY_BLOCK_MASK),ZoneNumber));
      
       /*Store Wear Count of all Free Blocks*/
       WearCountArray[idxWL] = spareArea.WearLevel;
       
       /*Store LUT index of free block*/
       ArrayIndex[idxWL] = pCurrentBlock;
       idxWL++;
       count++;    
      }
  }
 
  for ( loopVar1 = 0 ; loopVar1 < count-1; loopVar1++)
  {
    for ( loopVar2 = 0 ; loopVar2 < count-loopVar1-1 ; loopVar2++)
    {  
      
      cond1 =  (WearCountArray[loopVar2] != 0xFFFF);
      cond2 =  ((cond1)&& (WearCountArray[loopVar2+1] == 0xFFFF));
      cond3 = cond1&&(WearCountArray[loopVar2] > WearCountArray[loopVar2+1]);
      
       if(cond2 ||cond3)     /*0xFFFF ->Unused Block*/
        {                                                    
         swapSort = LUT[ArrayIndex[loopVar2]];
         LUT[ArrayIndex[loopVar2]] = LUT[ArrayIndex[loopVar2+1]];
         LUT[ArrayIndex[loopVar2+1]] = swapSort;  
          
         temp = WearCountArray[loopVar2];
         WearCountArray[loopVar2] = WearCountArray[loopVar2+1];
         WearCountArray[loopVar2+1]  = temp;             
       }
    }
  }   
  
 return NAND_OK;
}

/**
* @brief  NAND_GetFreeBlock.
*         Look for a free Block for data exchange from Look Up Table.
* @param  None
* @retval Logical Block Number of free Block.
*/
uint16_t NAND_GetFreeBlock (void)
{ 
  static uint16_t pCurrentBlock = 0;
  
  for ( ; pCurrentBlock < MaxLogBlockPerZone;\
    pCurrentBlock++)
  {
    
    if ((!((LUT[pCurrentBlock] & USED_BLOCK) == USED_BLOCK)) &&\
      (!((LUT[pCurrentBlock] & BAD_BLOCK) == BAD_BLOCK)))
    {
      NewIndex  = pCurrentBlock;      
      pCurrentBlock++; 
      if(pCurrentBlock >= MaxLogBlockPerZone)
        pCurrentBlock = 0;
      
      return LUT[NewIndex]& ((0xFFFF << 16) |(~(USED_BLOCK | VALID_BLOCK)));
    }   
  }
  NewIndex  = pCurrentBlock; 
  pCurrentBlock++;
  if(pCurrentBlock >= MaxLogBlockPerZone)
    pCurrentBlock = 0; 
  
  return LUT[NewIndex]& ((0xFFFF << 16) |(~(USED_BLOCK | VALID_BLOCK)));
  
}

/**
* @brief  SBLK_NAND_ReadSpareArea.
* @param  address: Corresponding Page Number of Spare Area to be read.
*                  Page Number in multiple of 512 Byte per Page. 
* @retval SPARE AREA after reading.
*/
SPARE_AREA SBLK_NAND_ReadSpareArea (uint32_t address)
{
  SPARE_AREA temp;
  uint8_t buffer[16];
  NAND_ADDRESS address_s;
  address_s = NAND_ConvertPhyAddress(address);
  FSMC_NAND_ReadSpareArea(buffer , address_s, 1) ; 
  temp = *(SPARE_AREA *)buffer;
  return temp;
}

/**
* @brief  LBLK_NAND_ReadSpareArea.
* @param  address: Corresponding Page Number of Spare Area to be read.
*                  Page Number in multiple of 512 Byte per Page. 
* @retval LBLK_SPARE_AREA after reading.
*/
LBLK_SPARE_AREA LBLK_NAND_ReadSpareArea (uint32_t address)
{
  LBLK_SPARE_AREA temp;
  uint8_t buffer[16];
  NAND_ADDRESS address_s;
  address_s = NAND_ConvertPhyAddress(address);
  FSMC_NAND_ReadSpareArea(buffer , address_s, 1);
  temp = *(LBLK_SPARE_AREA *)buffer;
  return temp;
  
}
/**
* @brief  WriteSpareArea.
* @param  address: Corresponding Page Number of Spare Area to be read.
*                  Page Number in multiple of 512 Byte.
** @param buff: Pointer to the data to be written in SPARE AREA.
* @retval Status of WriteSpareArea.  This Parameter can be 
*            NAND_OK:   when Write SPARE AREA  is successful. 
*            NAND_FAIL: when Write SPARE AREA fails to Write.
*/
uint16_t WriteSpareArea (uint32_t address,uint8_t *buff)
{
  NAND_ADDRESS address_s;
  address_s = NAND_ConvertPhyAddress(address);
  FSMC_NAND_WriteSpareArea(buff , address_s, 1) ;
  return NAND_OK;
}

/**
* @brief  NAND_Copy.
*         Copy pages from source to Destination.
* @param  Address_Src: Source Address.
* @param  Address_Dest: Destination Address.
* @param  PageToCopy: Number of Page to copy.
* @retval Status of NAND Copy . This Parameter can be 
*            NAND_OK: when the NAND copy is successful 
*            NAND_FAIL: when NAND fails to copy.
*/
uint16_t NAND_Copy (NAND_ADDRESS Address_Src,
                    NAND_ADDRESS Address_Dest, 
                    uint16_t PageToCopy)
{
  switch(NAND_TYPE)
  {
  case (SBLK_NAND):
    
        for ( ; PageToCopy > 0 ; PageToCopy-- )
        {
          FSMC_NAND_ReadSmallPage  ((uint8_t *)Buff, Address_Src , 1 );
          FSMC_NAND_WriteSmallPage ((uint8_t *)Buff, Address_Dest, 1);
          
          FSMC_NAND_ReadSpareArea((uint8_t *)Buff, Address_Src, 1);
          FSMC_NAND_WriteSpareArea((uint8_t *)Buff,Address_Dest, 1);      
          
          FSMC_NAND_AddressIncrement(&Address_Src);
          FSMC_NAND_AddressIncrement(&Address_Dest);
        }
        
      break;
  
  case(LBLK_NAND):
        for ( ; PageToCopy > 0 ; PageToCopy-- )
        {
          FSMC_NAND_ReadSmallPage  ((uint8_t *)Buff, Address_Src , 1 );
          FSMC_NAND_WriteSmallPage ((uint8_t *)Buff, Address_Dest, 1);
          
          FSMC_NAND_ReadSpareArea((uint8_t *)Buff, Address_Src, 1);
          FSMC_NAND_WriteSpareArea((uint8_t *)Buff,Address_Dest, 1);      
          
          FSMC_NAND_AddressIncrement(&Address_Src);
          FSMC_NAND_AddressIncrement(&Address_Dest);
        }
       
      break;
 }
 return NAND_OK;
}


/**
* @brief  NAND_CopyBack.
*         Copy pages from Source to Destination.
*         (Source & Destination address must have same page number).
* @param  Address_Src: Source Address.
* @param  Address_Dest: Destination Address.
* @param  PageToCopy: Number of Page to copy
* @retval Status of NAND Copy . This Parameter can be 
*            NAND_OK: when the NAND copy is successful 
*            NAND_FAIL: when NAND fails to copy.
*/
 uint16_t NAND_CopyBack (NAND_ADDRESS Address_Src,
                               NAND_ADDRESS Address_Dest, 
                               uint16_t PageToCopy)
 {
   uint16_t startPartialPage,endPartialPage,fullPage;
   uint16_t count;
   uint16_t loop;
   switch(NAND_TYPE)
   {
     case (SBLK_NAND):
     
       for ( ; PageToCopy > 0 ; PageToCopy-- )
       {
         FSMC_SBLK_NAND_CopyBack(Address_Src,Address_Dest);       
         FSMC_NAND_AddressIncrement(&Address_Src);
         FSMC_NAND_AddressIncrement(&Address_Dest);
       }
     
      break;
       
     case(LBLK_NAND):
        endPartialPage  = (Address_Src.Page + PageToCopy)%Multiplier;
        startPartialPage = (Address_Src.Page)%Multiplier;
        
        if(startPartialPage)
            startPartialPage = Multiplier - startPartialPage;

        if(startPartialPage >= PageToCopy)
        {
          startPartialPage = PageToCopy;
          endPartialPage = 0;
        }

         fullPage = (PageToCopy 
                      - startPartialPage 
                      - endPartialPage)/Multiplier;
       
       /*****Copy Partial pages(512B) before fullpage(ACTUAL PAGE SIZE)******
       ***It will range from 0 to (ACTUAL PAGE SIZE)/512B*/
       for(count = 0;count<startPartialPage;count++)
       {
         FSMC_NAND_ReadSmallPage  ((uint8_t *)Buff, Address_Src , 1);
         FSMC_NAND_WriteSmallPage ((uint8_t *)Buff, Address_Dest, 1);
         
         FSMC_NAND_ReadSpareArea((uint8_t *)Buff, Address_Src, 1);
         FSMC_NAND_WriteSpareArea((uint8_t *)Buff,Address_Dest, 1);      
         
         FSMC_NAND_AddressIncrement(&Address_Src);
         FSMC_NAND_AddressIncrement(&Address_Dest);         
       }
       
       /*****Copy Back Full pages(2048B)******/
       for(count = 0;count<fullPage;count++)
       {
         FSMC_LBLK_NAND_CopyBack(Address_Src,Address_Dest); 
         
         for(loop = 0;loop<Multiplier;loop++)
         {
           FSMC_NAND_AddressIncrement(&Address_Src);
           FSMC_NAND_AddressIncrement(&Address_Dest);
         } 
       } 
       
       /*****Copy Partial pages(512B) after fullpage(ACTUAL PAGE SIZE)******
       **** It will range from 0 to (ACTUAL PAGE SIZE)/512B*/
       
       for(count = 0;count<endPartialPage;count++)
       {
         FSMC_NAND_ReadSmallPage  ((uint8_t *)Buff, Address_Src , 1);
         FSMC_NAND_WriteSmallPage ((uint8_t *)Buff, Address_Dest, 1);
         
         FSMC_NAND_ReadSpareArea((uint8_t *)Buff, Address_Src, 1);
         FSMC_NAND_WriteSpareArea((uint8_t *)Buff,Address_Dest, 1);      
         
         FSMC_NAND_AddressIncrement(&Address_Src);
         FSMC_NAND_AddressIncrement(&Address_Dest);          
       }           
       break;
   }
   return NAND_OK;
 }
                    
/**
* @brief  NAND_Format.
*         Format the entire NAND flash.
* @retval Status of NAND Format . This Parameter can be 
*            NAND_OK: when the NAND Format is successful 
*            NAND_FAIL: when NAND fails to Format.
*/
uint16_t NAND_Format (void)
{
  NAND_ADDRESS phAddress;
  uint32_t blockIndex;
  uint8_t zone =0;
  switch(NAND_TYPE)
  {
  case (SBLK_NAND):     
    for(zone = 0;zone<MaxZone;zone++) 
    {
      for(blockIndex = 0;blockIndex<MaxPhyBlockPerZone;blockIndex++)
      {
        phAddress = NAND_ConvertPhyAddress(SBLK_PAGE_NUMBER(blockIndex,zone));
        /***Erase the Block**/
        FSMC_NAND_EraseBlock (phAddress); 
      }
    }
    break;
   case(LBLK_NAND):
     for(zone = 0;zone<MaxZone;zone++) 
     {
       for(blockIndex = 0;blockIndex<MaxPhyBlockPerZone;blockIndex++)
       {
         phAddress = NAND_ConvertPhyAddress(LBLK_PAGE_NUMBER(blockIndex,zone));
         /****Erase the Block****/    
         FSMC_NAND_EraseBlock (phAddress);
       }
     }
     break;
  }
  NAND_BuildLUT(0);
  return NAND_OK;
}

/**
* @brief  NAND_PostWrite.
*         NAND Post Write.
* @param  None
* @retval Status of NAND Post Write . This Parameter can be 
*            NAND_OK: when the NAND Post Write is successful 
*            NAND_FAIL: when NAND fails to Post Write.
*/
uint16_t NAND_PostWrite (void)
{ 
  switch(NAND_TYPE)
  {
      case (SBLK_NAND):
        SBLK_NAND_PostWrite();
        break;   
      case(LBLK_NAND):
        LBLK_NAND_PostWrite();
        break;
  }
  return NAND_OK;
}

/**
* @brief  SBLK_NAND_PostWrite.
*         Small Block NAND_PostWrite.
* @param  None
* @retval None
*/
void SBLK_NAND_PostWrite (void)
{ 
  uint32_t swapIdx;
  uint16_t  tempSpareArea [8];
  uint16_t  pageBack;
  uint32_t  swapVariable;
  NAND_ADDRESS tempNandAddress;
  
  if ( Block_State == OLD_BLOCK )
  {
      /* Precopy Old first pages */
    if (InitialPage != 0)
    {
      pageBack = WAddress.Page;
      FAddress.Page = WAddress.Page = 0;
      
      FSMC_NAND_ReadSmallPage  ((uint8_t *)Buff, WAddress , 1 );
      FSMC_NAND_WriteSmallPage ((uint8_t *)Buff, FAddress, 1);
      
      FSMC_NAND_ReadSpareArea((uint8_t *)tempSpareArea, WAddress, 1);
      
      tempSpareArea [0] = 0xFFFF;
      tempSpareArea [1] = 0xFFFF;
      tempSpareArea [2] = 0xFFFF; 
      tempSpareArea [3] = 0xFFFF; 
      tempSpareArea [4] = 0xFFFF; 
      tempSpareArea [5] = 0xFFFF; 
      
      FSMC_NAND_WriteSpareArea((uint8_t *)tempSpareArea,FAddress, 1);          
      
      FSMC_NAND_AddressIncrement(&WAddress);
      FSMC_NAND_AddressIncrement(&FAddress);
      
      
      if(COPY_STATE == COPY_BACK_SUPPORTED)
        NAND_CopyBack(WAddress, FAddress, InitialPage - 1 );
      else
        NAND_Copy(WAddress, FAddress, InitialPage - 1 );
      WAddress.Page =  pageBack ;
      
    }

        /* Postcopy remaining pages */
      if ((SBLK_NAND_BLOCK_SIZE - (WAddress.Page + 1)) != 0)
      {
        FSMC_NAND_AddressIncrement(&WAddress);
        FAddress.Page = WAddress.Page;
       if(COPY_STATE == COPY_BACK_SUPPORTED)
        NAND_CopyBack(WAddress, FAddress, SBLK_NAND_BLOCK_SIZE - WAddress.Page); 
       else
        NAND_Copy(WAddress, FAddress, SBLK_NAND_BLOCK_SIZE - WAddress.Page); 
      }
      
      tempSpareArea [0] = Swap((LogAddress << 1)| (GetParity(LogAddress)));
      tempSpareArea [1] = 0xFFFF;
      tempSpareArea [2] = 0xFFFF; 
      tempSpareArea [3] = 0xFFFF; 
      tempSpareArea [4] = 0xFFFF; 
      tempSpareArea [5] = 0xFFFF; 
      tempSpareArea [6] = 0xFFFF; 
      tempSpareArea [7] = 0xFFFF;   
      
      FAddress.Page     = 0x00;
      
      FSMC_NAND_WriteSpareArea( (uint8_t *)tempSpareArea , FAddress , 1);
      
      swapIdx = LUT[OldIndex] & 0x03FF;
      swapVariable  = LUT[OldIndex];
      LUT[OldIndex] = LUT[NewIndex];
      LUT[NewIndex] = swapVariable;
      
      /*Read the SPARE AREA before Erasing the OLD Block*/
      tempNandAddress = WAddress;
      tempNandAddress.Page = 0;
      FSMC_NAND_ReadSpareArea( (uint8_t *)tempSpareArea , tempNandAddress , 1);
      
       WAddress.Page = 0;  
       
       /*Erase the Block*/
      FSMC_NAND_EraseBlock(WAddress);

      tempSpareArea [0] = 0xFFFF;
      tempSpareArea [1] += 2;
      tempSpareArea [2] = 0xFFFF; 
      tempSpareArea [3] = 0xFFFF; 
      tempSpareArea [4] = 0xFFFF; 
      tempSpareArea [5] = 0xFFFF; 
      tempSpareArea [6] = 0xFFFF; 
      tempSpareArea [7] = 0xFFFF;   
      
      /* Write the Spare Area on 0th page of the Erased Block*/
      FSMC_NAND_WriteSpareArea( (uint8_t *)tempSpareArea , tempNandAddress ,1);
      
      /**Update LUT**/
      LUT[NewIndex] = (uint32_t)(0xFFFF<<16)|swapIdx;
      LUT[OldIndex] ^= (USED_BLOCK |VALID_BLOCK);
  }
  else     /* Unused Block case */
  {
      tempSpareArea [0] = Swap((LogAddress << 1)| (GetParity(LogAddress)));
      tempSpareArea [1] = 0xFFFF;
      tempSpareArea [2] = 0xFFFF; 
      tempSpareArea [3] = 0xFFFF; 
      tempSpareArea [4] = 0xFFFF; 
      tempSpareArea [5] = 0xFFFF; 
      tempSpareArea [6] = 0xFFFF; 
      tempSpareArea [7] = 0xFFFF;     
      
      WAddress.Page     = 0x00;
      
      FSMC_NAND_WriteSpareArea((uint8_t *)tempSpareArea , WAddress, 1);
      
      LUT[LogAddress] |= (0xFFFF<<16)|WAddress.Block| USED_BLOCK| VALID_BLOCK; 
   }

}

/**
* @brief  LBLK_NAND_PostWrite.
*         Large Block NAND Post Write.
* @param  None
* @retval None
*/
void LBLK_NAND_PostWrite (void)
{ 
  uint32_t  swapIdx;
  uint16_t  tempSpareArea [8];
  uint16_t  pageBack;
  uint32_t  swapVariable;
  NAND_ADDRESS tempNandAddress;
  
  if ( Block_State == OLD_BLOCK )
  {
    /* Precopy Old first pages */
    if (InitialPage != 0)
    {
      pageBack = WAddress.Page;                     
      FAddress.Page = WAddress.Page = 0;
      
      
      FSMC_NAND_ReadSmallPage  ((uint8_t *)Buff, WAddress , 1 );
      FSMC_NAND_WriteSmallPage ((uint8_t *)Buff, FAddress, 1);
      
      FSMC_NAND_ReadSpareArea((uint8_t *)tempSpareArea, WAddress, 1);
      
      tempSpareArea [0] = 0xFFFF;
      tempSpareArea [1] = 0xFFFF;
      tempSpareArea [2] = 0xFFFF; 
      tempSpareArea [3] = 0xFFFF; 
      tempSpareArea [4] = 0xFFFF; 
      tempSpareArea [5] = 0xFFFF; 
      
      FSMC_NAND_WriteSpareArea((uint8_t *)tempSpareArea,FAddress, 1);          
      
      FSMC_NAND_AddressIncrement(&WAddress);
      FSMC_NAND_AddressIncrement(&FAddress);
      
      
      if(COPY_STATE == COPY_BACK_SUPPORTED)
        NAND_CopyBack(WAddress, FAddress, InitialPage - 1);
      else
        NAND_Copy(WAddress, FAddress, InitialPage - 1);
      WAddress.Page =  pageBack ;
      
    }

    /* Postcopy remaining pages */
    if ((NandBlockSize - (WAddress.Page + 1)) != 0)
    {
      FSMC_NAND_AddressIncrement(&WAddress);
      FAddress.Page = WAddress.Page;
      
       if(COPY_STATE == COPY_BACK_SUPPORTED)
        NAND_CopyBack(WAddress, FAddress, NandBlockSize - WAddress.Page);
      else
        NAND_Copy(WAddress, FAddress, NandBlockSize - WAddress.Page);
      
    }
    
    tempSpareArea [0] = 0xFFFF;
    tempSpareArea [1] = Swap((LogAddress << 1)| (GetParity(LogAddress)));
    tempSpareArea [2] = 0xFFFF; 
    tempSpareArea [3] = 0xFFFF; 
    tempSpareArea [4] = 0xFFFF; 
    tempSpareArea [5] = 0xFFFF; 
    tempSpareArea [6] = 0xFFFF; 
    tempSpareArea [7] = 0xFFFF;   
    
    FAddress.Page     = 0x00;
    FSMC_NAND_WriteSpareArea( (uint8_t *)tempSpareArea , FAddress , 1);
    
    swapIdx = LUT[OldIndex] & 0x03FF;
    swapVariable  = LUT[OldIndex];
    LUT[OldIndex] = LUT[NewIndex];
    LUT[NewIndex] = swapVariable;
    
    /*Read the SPARE AREA before Erasing the OLD Block*/
    tempNandAddress = WAddress;
    tempNandAddress.Page = 0;
    FSMC_NAND_ReadSpareArea( (uint8_t *)tempSpareArea , tempNandAddress , 1);
    
     WAddress .Page = 0;
      /*Erase the Block*/
    FSMC_NAND_EraseBlock(WAddress);

    tempSpareArea [0] = 0xFFFF;
    tempSpareArea [1] = 0xFFFF;
    tempSpareArea [2] += 2;
    tempSpareArea [3] = 0xFFFF; 
    tempSpareArea [4] = 0xFFFF; 
    tempSpareArea [5] = 0xFFFF; 
    tempSpareArea [6] = 0xFFFF; 
    tempSpareArea [7] = 0xFFFF;  
    
    /* Write the Spare Area on 0th page of the Erased Block*/
    FSMC_NAND_WriteSpareArea( (uint8_t *)tempSpareArea , tempNandAddress ,1);
    
    /**Update LUT**/
    LUT[NewIndex] = (uint32_t)(0xFFFF<<16)|swapIdx;
    LUT[OldIndex] ^= (USED_BLOCK |VALID_BLOCK);
  }
  else     /* Unused Block case */
  {
    tempSpareArea [0] = 0xFFFF;
    tempSpareArea [1] = Swap((LogAddress << 1)| (GetParity(LogAddress)));
    tempSpareArea [2] = 0xFFFF; 
    tempSpareArea [3] = 0xFFFF; 
    tempSpareArea [4] = 0xFFFF; 
    tempSpareArea [5] = 0xFFFF; 
    tempSpareArea [6] = 0xFFFF; 
    tempSpareArea [7] = 0xFFFF;     
    
    WAddress.Page     = 0x00;
    
    FSMC_NAND_WriteSpareArea((uint8_t *)tempSpareArea , WAddress, 1);
    
    LUT[LogAddress] |= (0xFFFF<<16)|WAddress.Block| USED_BLOCK| VALID_BLOCK; 
  }

}

/**
* @brief  NAND_GarbageCollection.
*         Erase of Blocks every time the write operation is stopped.
* @param  None
* @retval Status of NAND Garbage collection . This Parameter can be 
*            NAND_OK: when the NAND Garbage collection is successful 
*            NAND_FAIL: when NAND fails to Garbage collection.
*/
uint16_t NAND_GarbageCollection(void)
{ 
  uint16_t i=0;
  /*Erase blocks of Victim buffer every time the flash is not busy*/
  if (Write_State == WRITE_IDLE)
  {
    for (i=0 ; i < VictimBlocks  ; i++)
    {
      NAND_UpdateWearLevelCounter (Victim[i]);
    }
    VictimIdx=0;
    VictimBlocks=0;
  } 
  return NAND_OK;
}

/**
* @brief  NAND_UpdateWearLevelCounter.
*         Increment the value of Wear Level counter after every erase.
* @param  Address: Logical Address.
* @retval Status of NAND Update Wear Level. This Parameter can be 
*            NAND_OK: when the NAND Update Wear Level is successful 
*            NAND_FAIL: when NAND fails to Update Wear Level.
*/
uint16_t NAND_UpdateWearLevelCounter ( NAND_ADDRESS Address)
{   
  uint16_t i;
  SPARE_AREA  SpareArea;
  
  /*Read the Spare Area*/
  SpareArea = SBLK_NAND_ReadSpareArea (Address.Block);
  /*Erase the Block*/
  FSMC_NAND_EraseBlock(Address);
  /*Increment the value of the Wear Level 
        and write the spare area in the Erased Block*/
  SpareArea.WearLevel++;
  SpareArea.LogicalIndex = 0xFFFF;
  SpareArea.DataStatus = 0xFF;
  SpareArea.BlockStatus = 0xFF;
  SpareArea.ECC = 0xFFFFFFFF;
  for (i = 0 ; i < 6 ; i++ )
  {
    SpareArea.reserved[i] = 0xFF;
  } 
  Address.Page     = 0x00;
  FSMC_NAND_WriteSpareArea((uint8_t *)&SpareArea , Address, 1);
  
  /*Choose the next Block to write using the wear leveling method*/
  NAND_CleanLUT (Address.Zone);
  
  return NAND_OK;
}
/**
* @brief  NAND_ConvertPhyAddress.
*          Convert Memory Offset into Physical Address.
* @param   Address: Memory Offset in Multiple of 512B(0,512/512,1024/512...).
* @retval  Physycal Address.
*/
NAND_ADDRESS NAND_ConvertPhyAddress (uint32_t Address)
{
  NAND_ADDRESS Address_t = {0};
  
  switch(NAND_TYPE)
  {
    case(SBLK_NAND):
          Address_t.Page  = Address & (SBLK_NAND_BLOCK_SIZE - 1);
          Address_t.Block = Address / SBLK_NAND_BLOCK_SIZE;
          Address_t.Zone = 0;
          
          while (Address_t.Block >= MaxPhyBlockPerZone) 
          {
            Address_t.Block -= MaxPhyBlockPerZone;  
            Address_t.Zone++;
          }
        break;
        
    case(LBLK_NAND):
          Address_t.Page  = Address & (NandBlockSize - 1);
          Address_t.Block = Address / NandBlockSize;
          Address_t.Zone = 0;
          
          while (Address_t.Block >= MaxPhyBlockPerZone)
          {
            Address_t.Block -= MaxPhyBlockPerZone;
            Address_t.Zone++;
          }
        break;   
  }
  return Address_t;
}

/**
* @brief  NAND_BuildLUT.
*         Build the Look Up Table.
* @param  ZoneNbr: The Zone Number.
* @retval Status of NAND Build Look Up Table. This Parameter can be 
*            NAND_OK: when the NAND Build Look Up Table is successful 
*            NAND_FAIL: when NAND fails to Build Look Up Table.
*/
uint16_t NAND_BuildLUT (uint8_t Zone)
{
  switch(NAND_TYPE)
  {
   case (SBLK_NAND):
      SBLK_NAND_BuildLUT(Zone);
      break;
   case(LBLK_NAND):
      LBLK_NAND_BuildLUT(Zone);
      break;  
  }
  return NAND_OK;
}


/**
* @brief  SBLK_NAND_BuildLUT.
*          Build the Look Up Table.
* @param   ZoneNbr: The Zone Number.
* @retval Status of NAND Build Look Up Table. This Parameter can be 
*            NAND_OK: when the NAND Build Look Up Table is successful 
*            NAND_FAIL: when NAND fails to Build Look Up Table.
* !!!! NOTE : THIS ALGORITHM IS A SUBJECT OF PATENT FOR STMICROELECTRONICS !!!!!
*/

uint16_t SBLK_NAND_BuildLUT (uint8_t ZoneNumber)
{  
  uint16_t logicalIdx,physicalIdx;
  uint16_t  pBadBlock ,index;
  SPARE_AREA  SpareArea;
  uint16_t temp;
    uint16_t count;
  
  for(index = 0; index < MaxPhyBlockPerZone ; index++)
    LUT[index] = 0;
  
  /* Init Bad Block Pointer */
  pBadBlock  = MaxPhyBlockPerZone - 1;
  
  for(index = 0; index < MaxPhyBlockPerZone ; index++)
  {
     SpareArea = SBLK_NAND_ReadSpareArea(SBLK_PAGE_NUMBER(index,ZoneNumber));
     
    /*******if it's found to be BAD BLOCK********/
    if ((SpareArea.DataStatus == 0) || (SpareArea.BlockStatus == 0) ||\
      (GetParity(Swap(SpareArea.LogicalIndex))!=0))
    { 
      LUT[pBadBlock--]  |= (0xFFFF << 16) | index | (uint16_t) BAD_BLOCK ;
      
     /* pBadBlock should not cross MAX_LOG_BLOCKS_PER_ZONE*/
      if (pBadBlock == MaxLogBlockPerZone) 
      {
        return NAND_FAIL;
      }
    }
    
   /********if it's found to be USED and VALID BLOCKS*********/
    else if (SpareArea.LogicalIndex != 0xFFFF)
    {
      temp = (Swap(SpareArea.LogicalIndex) >>1) & 0x3FF;
      LUT[temp] |= (0xFFFF << 16)| index |VALID_BLOCK | USED_BLOCK;
    }      
  }
  
  /********************* Locate FREE BLOCKS ******************/
  logicalIdx = 0;
  
  physicalIdx = 0;
  
  for(count = 0;count<MaxPhyBlockPerZone;count++)
  {
    
    if(LUT[logicalIdx] == 0)    /* If Logical Block not assigned yet */
    {
      for(;physicalIdx<MaxPhyBlockPerZone;physicalIdx++)
      { 
        SpareArea = SBLK_NAND_ReadSpareArea(SBLK_PAGE_NUMBER\
                                            (physicalIdx,ZoneNumber));
        
        /******* Check Spare Area for FREE BLOCKS *****/
        if ((SpareArea.DataStatus == 0xFF) && \
          (SpareArea.LogicalIndex == 0xFFFF)&&\
            (GetParity(Swap(SpareArea.LogicalIndex))==0))
          {
            LUT[ logicalIdx ] |= (0xFFFF << 16)| physicalIdx;
            physicalIdx++;
            logicalIdx++;
            break;
          }
      }
    }
    else
      logicalIdx++;  
  }
  return NAND_OK;
}
/**
* @brief  LBLK_NAND_BuildLUT.
*          Build the Look Up Table.
* @param  ZoneNbr: The Zone Number.
* @retval Status of NAND Build Look Up Table. This Parameter can be 
*             NAND_OK: when the NAND Build Look Up Table is successful 
*            NAND_FAIL: when NAND fails to Build Look Up Table.
* !!!! NOTE : THIS ALGORITHM IS A SUBJECT OF PATENT FOR STMICROELECTRONICS !!!!!
*/

uint16_t LBLK_NAND_BuildLUT (uint8_t ZoneNbr)
{  
  uint16_t logicalIdx,physicalIdx;
  uint16_t  pBadBlock ,index;
  LBLK_SPARE_AREA  lblkSpareArea;
  uint16_t temp;
  uint16_t count;
  
  for(index = 0; index < MaxPhyBlockPerZone ; index++)
    LUT[index] = 0;
  
  /* Init Bad Block Pointer */
  pBadBlock  = MaxPhyBlockPerZone - 1;
  
  for(index = 0; index < MaxPhyBlockPerZone ; index++)
  {
     lblkSpareArea = LBLK_NAND_ReadSpareArea(LBLK_PAGE_NUMBER(index,ZoneNbr));
     
    /*******if it's found to be BAD BLOCK********/
    if ((lblkSpareArea.DataStatus == 0) || (lblkSpareArea.BlockStatus == 0) || \
      (GetParity(Swap(lblkSpareArea.LogicalIndex))!=0))
    { 
      LUT[pBadBlock--]  |= (0xFFFF << 16) | index | (uint16_t) BAD_BLOCK ;
      
    /* pBadBlock should not cross MAX_LOG_BLOCKS_PER_ZONE*/
      if (pBadBlock == MaxLogBlockPerZone) 
      {
        return NAND_FAIL;
      }
    }
    
   /********if it's found to be USED and VALID BLOCKS*********/
    else if (lblkSpareArea.LogicalIndex != 0xFFFF)
    {
      temp = (Swap(lblkSpareArea.LogicalIndex) >>1) & 0x3FF;
      LUT[temp] |= (0xFFFF << 16)| index |VALID_BLOCK | USED_BLOCK;
    }      
  }
  
  /********************* Locate FREE BLOCKS ******************/
  logicalIdx = 0;
  physicalIdx = 0;
  
  for(count = 0;count<MaxPhyBlockPerZone;count++)
  {
    
    if(LUT[logicalIdx] == 0)    /* If Logical Block not assigned yet */
    {
      for(;physicalIdx<MaxPhyBlockPerZone;physicalIdx++)
      {
          lblkSpareArea = LBLK_NAND_ReadSpareArea(\
            LBLK_PAGE_NUMBER(physicalIdx,ZoneNbr));
         
        /******* Check Spare Area for FREE BLOCKS *****/
        if ((lblkSpareArea.DataStatus == 0xFF) && \
          (lblkSpareArea.LogicalIndex == 0xFFFF)&&\
            (GetParity(Swap(lblkSpareArea.LogicalIndex))==0))
          {
            LUT[ logicalIdx ] |= (0xFFFF << 16)| physicalIdx;
            physicalIdx++;
            logicalIdx++;
            break;
          }
      }
    }
    else
      logicalIdx++;  
  }
  return NAND_OK;
}

/**
* @brief  GetParity.
*          Calculate parity.
* @param  in_value: 16-bit value.
* @retval Status.
*/
 uint8_t GetParity (uint16_t in_value)
{
  uint8_t count=0, rtrn =0;
  for(count = 0; count < 16; ++count)
  {
    if( (in_value>>count)&1 )
    {
      rtrn^=1;
    }
  } 
  return rtrn;
}

/**
* @brief  swap.
*          swap a 16 bit.
* @param  in : 16-bit value.
* @retval swapped value.
*/

uint16_t Swap (uint16_t in)
{
  unsigned char var1, var2;
  
  var1 = in & 0xFF;
  var2 = ( in >> 8 ) & 0xFF;
  
  return (var1 << 8) + var2; 
} 

/**
* @brief  WritePage.
*          Write a page & Corresponding SPARE AREA.
* @param  Address : The addrees of the page to write.
* @param  *buff : The buffer to write in.
* @param  len : The Number of page to write.
* @retval None
*/
void NAND_WritePage(NAND_ADDRESS Address , uint8_t *buff , uint16_t len)
{
  switch(NAND_TYPE)
  {
    case(SBLK_NAND):
        SBLK_NAND_WritePage(Address,(uint8_t *)buff,len);
       break;
      
    case(LBLK_NAND):
        LBLK_NAND_WritePage(Address,(uint8_t *)buff,len);
       break;
  }
}

/**
* @brief  SBLK_NAND_WritePage.
*         Write a page & Corresponding ECC in SPARE AREA in Small Block NAND.
* @param  Address : The addrees of the page to write.
* @param  *buff : The buffer to write in.
* @param  len : The Number of page to write.
* @retval None
*/
void SBLK_NAND_WritePage(NAND_ADDRESS Address , uint8_t *buff , uint16_t len)
{
  
  uint32_t ECC_code;
  SPARE_AREA  tempSpareArea ;
  uint8_t count = 0;
  
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);  
  FSMC_NANDECCCmd(FSMC_BANK_NAND, ENABLE);
  FSMC_NAND_WriteSmallPage( (uint8_t *)buff , Address, len);
  ECC_code =  FSMC_GetECC(FSMC_BANK_NAND);
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);        

  tempSpareArea.LogicalIndex = 0xFFFF;             
  tempSpareArea.WearLevel = 0xFFFF; 
  tempSpareArea.DataStatus = 0xFF;
  tempSpareArea.BlockStatus = 0xFF;
  tempSpareArea.ECC = ECC_code;
  
  for (count = 0 ; count < 6 ; count++ )
  {
    tempSpareArea.reserved[count] = 0xFF;
  }
  FSMC_NAND_WriteSpareArea( (uint8_t *)&tempSpareArea , Address , 1); 
  
}


/**
* @brief  LBLK_NAND_WritePage.
*          Write a page & Corresponding ECC in SPARE AREA in Small Block NAND.
* @param  Address : The addrees of the page to write.
* @param  *buff : The buffer to write in.
* @param  len : The Number of page to write.
* @retval None
*/

void LBLK_NAND_WritePage(NAND_ADDRESS Address , uint8_t *buff , uint16_t len)
{
  uint32_t ECC_code;
  LBLK_SPARE_AREA  largeSpareArea ;
  uint8_t count = 0;
  
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);  
  FSMC_NANDECCCmd(FSMC_BANK_NAND, ENABLE);
  FSMC_NAND_WriteSmallPage( (uint8_t *)buff , Address, len);
  ECC_code =  FSMC_GetECC(FSMC_BANK_NAND);
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);        

  largeSpareArea.LogicalIndex = 0xFFFF;             
  largeSpareArea.WearLevel = 0xFFFF; 
  largeSpareArea.DataStatus = 0xFF;
  largeSpareArea.BlockStatus = 0xFF;
  largeSpareArea.ECC = ECC_code;
  
  for (count = 0 ; count < 6 ; count++ )
  {
    largeSpareArea.reserved[count] = 0xFF;
  }
  FSMC_NAND_WriteSpareArea( (uint8_t *)&largeSpareArea , Address , 1); 
}

/**
* @brief  ReadPage.
*          Read a page considering Error correction code(1 bit per 512 Byte).
* @param  Address : The addrees of the page to read.
* @param  *buff : The buffer to read from.
* @param  len : The number of page to read.
* @retval None
*/
void NAND_ReadPage (NAND_ADDRESS Address , uint8_t *buff , uint16_t len )
{ 
  switch(NAND_TYPE)
  {
    case(SBLK_NAND):
       SBLK_NAND_ReadPage(Address,(uint8_t *)buff,len);
      break;
    case(LBLK_NAND):
       LBLK_NAND_ReadPage(Address,(uint8_t *)buff,len);
       break;
  }
}

/**
* @brief  SBLK_NAND_ReadPage.
*          Read a page considering Error correction code(1 bit per 512 Byte)
*          in Small Block NAND.
* @param  Address : The addrees of the page to read.
* @param  *buff : The buffer to read from.
* @param  len : The number of page to read.
* @retval None
*/
void SBLK_NAND_ReadPage (NAND_ADDRESS Address , uint8_t *buff , uint16_t len )
{
  
  SPARE_AREA  tempSpareArea;  
  uint32_t ECC_code_stored = 0xFFFFFFFF;
  uint32_t ECC_code_new = 0xFFFFFFFF;
  uint32_t XOR_ECC =  0xFFFFFFFF ;
  uint8_t XOR_ECC_ONES;
  uint16_t XOR_ECC_1B,XOR_ECC_2B,XOR_ECC_3B;
  uint16_t bit_address = 0,byte_address = 0;
  
  /* Page size--------ECC_Code Size
  256---------------22 bits LSB  (ECC_CODE & 0x003FFFFF)
  512---------------24 bits      (ECC_CODE & 0x00FFFFFF)
  1024--------------26 bits      (ECC_CODE & 0x03FFFFFF)
  2048--------------28 bits      (ECC_CODE & 0x0FFFFFFF)
  4096--------------30 bits      (ECC_CODE & 0x3FFFFFFF)
  8192--------------32 bits      (ECC_CODE & 0xFFFFFFFF)
  */  
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);
  FSMC_NANDECCCmd(FSMC_BANK_NAND, ENABLE); 
  FSMC_NAND_ReadSmallPage ((uint8_t *)buff , Address, len);
  ECC_code_new =  FSMC_GetECC(FSMC_BANK_NAND);
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);
  
  /*For Page size 512, ECC_Code size 24 bits*/
  ECC_code_new = ECC_code_new & 0x00FFFFFF;      
  FSMC_NAND_ReadSpareArea((uint8_t *)&tempSpareArea, Address,1);
  ECC_code_stored  =  tempSpareArea.ECC;
  
  /*For Page size 512, ECC_Code size 24 bits*/
  ECC_code_stored  = ECC_code_stored & 0x00FFFFFF; 
  XOR_ECC = ECC_code_stored^ECC_code_new;    
  XOR_ECC_ONES = BitCount(XOR_ECC);
  
  XOR_ECC_1B = (XOR_ECC & 0x000000FF);          //1st Byte of ECC
  XOR_ECC_2B = ((XOR_ECC & 0x0000FF00)>>8);     //2nd Byte of ECC
  XOR_ECC_3B = ((XOR_ECC & 0x00FF0000)>>16);    //3rd Byte of ECC
  
  if(XOR_ECC == 0)          
  {       
    /********** No Error ********/
  }
  else
  {
    if( XOR_ECC_ONES < 23 )   
    {
      
        if(XOR_ECC_ONES == 12) 
        {
         /********  Correctable ERROR   *******/  
          bit_address =   ((XOR_ECC_1B >> 1) & 0x01) |\
                          ((XOR_ECC_1B >> 2) & 0x02) |\
                          ((XOR_ECC_1B >> 3) & 0x04);
          
          byte_address =  ((XOR_ECC_1B >> 7) & 0x01) |\
                          ((XOR_ECC_2B )     & 0x02) |\
                          ((XOR_ECC_2B >> 1) & 0x04) |\
                          ((XOR_ECC_2B >> 2) & 0x08) |\
                          ((XOR_ECC_2B >> 3) & 0x10) |\
                          ((XOR_ECC_3B << 4) & 0x20) |\
                          ((XOR_ECC_3B << 3) & 0x40) |\
                          ((XOR_ECC_3B << 2) & 0x80) |\
                          ((XOR_ECC_3B << 1) & 0x100);     
          
          /*Correct bit error in the data*/
         buff[byte_address] = (buff[byte_address])^((uint8_t )(1<<bit_address));
         ECC_CorrectableErrorFlag = 1;
        }
        else
        {
          /********Non _Correctable ERROR********/
        }
      
    }
    else
    {
      /*******ECC ERROR***********/
    }  
  }
}

/**
* @brief  LBLK_NAND_ReadPage.
*          Read a page considering Error correction code(1 bit per 512 Byte)
*          in Large Block NAND.
* @param  Address : The addrees of the page to read.
* @param  *buff : The buffer to read from.
* @param  len : The number of page to read.
* @retval None
*/
void LBLK_NAND_ReadPage (NAND_ADDRESS Address , uint8_t *buff , uint16_t len )
{
  LBLK_SPARE_AREA  largeSpareArea;  
  uint32_t ECC_code_stored,ECC_code_new,XOR_ECC;
  uint8_t XOR_ECC_ONES;
  uint16_t XOR_ECC_1B,XOR_ECC_2B,XOR_ECC_3B;
  uint16_t bit_address,byte_address;
  
  /* Page size--------ECC_Code Size
  256---------------22 bits LSB  (ECC_CODE & 0x003FFFFF)
  512---------------24 bits      (ECC_CODE & 0x00FFFFFF)
  1024--------------26 bits      (ECC_CODE & 0x03FFFFFF)
  2048--------------28 bits      (ECC_CODE & 0x0FFFFFFF)
  4096--------------30 bits      (ECC_CODE & 0x3FFFFFFF)
  8192--------------32 bits      (ECC_CODE & 0xFFFFFFFF)
  */  
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);
  FSMC_NANDECCCmd(FSMC_BANK_NAND, ENABLE); 
  FSMC_NAND_ReadSmallPage ((uint8_t *)buff , Address, len);
  ECC_code_new =  FSMC_GetECC(FSMC_BANK_NAND);
  FSMC_NANDECCCmd(FSMC_BANK_NAND, DISABLE);
  
  /*For Page size 512, ECC_Code size 24 bits*/
  ECC_code_new = ECC_code_new & 0x00FFFFFF;      
  FSMC_NAND_ReadSpareArea((uint8_t *)&largeSpareArea, Address,1);
  ECC_code_stored  =  largeSpareArea.ECC;
  
  /*For Page size 512, ECC_Code size 24 bits*/
  ECC_code_stored  = ECC_code_stored & 0x00FFFFFF; 
  XOR_ECC = ECC_code_stored^ECC_code_new;    
  XOR_ECC_ONES = BitCount(XOR_ECC);
  
  XOR_ECC_1B = (XOR_ECC & 0x000000FF);          //1st Byte of ECC
  XOR_ECC_2B = ((XOR_ECC & 0x0000FF00)>>8);     //2nd Byte of ECC
  XOR_ECC_3B = ((XOR_ECC & 0x00FF0000)>>16);    //3rd Byte of ECC
  
  /*********CASE of No ERROR ********/
  if(XOR_ECC == 0)          
  {       
    /********** No Error ********/
  }
  else
  {
    if( XOR_ECC_ONES < 23 )   
    {
      
        if(XOR_ECC_ONES == 12) 
        {
         /********  Correctable ERROR   *******/  
          bit_address =   ((XOR_ECC_1B >> 1) & 0x01) |\
                          ((XOR_ECC_1B >> 2) & 0x02) |\
                          ((XOR_ECC_1B >> 3) & 0x04);
          
          byte_address =  ((XOR_ECC_1B >> 7) & 0x01) |\
                          ((XOR_ECC_2B )     & 0x02) |\
                          ((XOR_ECC_2B >> 1) & 0x04) |\
                          ((XOR_ECC_2B >> 2) & 0x08) |\
                          ((XOR_ECC_2B >> 3) & 0x10) |\
                          ((XOR_ECC_3B << 4) & 0x20) |\
                          ((XOR_ECC_3B << 3) & 0x40) |\
                          ((XOR_ECC_3B << 2) & 0x80) |\
                          ((XOR_ECC_3B << 1) & 0x100);     
          
          /*Correct bit error in the data*/
         buff[byte_address] = (buff[byte_address])^((uint8_t )(1<<bit_address));
         ECC_CorrectableErrorFlag = 1;
        }
        else
        {
          /********Non _Correctable ERROR********/
        }
      
    }
    else
    {
      /*******ECC ERROR***********/
    }  
  }
}

/**
* @brief  BitCount.
*          Counts the number of 1's in 32 bit Number.
* @param  num : The number in which number of 1's to be counted.
* @retval The number of one in 32 bit number.
*/
uint8_t BitCount(uint32_t num)
{
  uint8_t count,ones = 0;
  uint32_t temp = 0x01,temp1;
  
  for(count=0;count<32;count++)
  {
    temp1 = temp<<count;
    if((num&temp1)  == temp1 )
      ones++;  
  }
  return ones;
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

void FSMC_NANDECCCmd(uint32_t FSMC_Bank, FunctionalState NewState)
{
  assert_param(IS_FSMC_NAND_BANK(FSMC_Bank));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  FSMC_NAND_TypeDef *device;
  device = FSMC_Bank2_3;

  if (NewState != DISABLE)
  {
    /* Enable the selected NAND Bank ECC function by setting the ECCEN bit in the PCRx register */

      device->PCR2 |= PCR_ECCEN_SET;

  }
  else
  {
    /* Disable the selected NAND Bank ECC function by clearing the ECCEN bit in the PCRx register */
      device->PCR2 &= PCR_ECCEN_RESET;

}
}

uint32_t FSMC_GetECC(uint32_t FSMC_Bank)
{
	FSMC_Bank++;
	FSMC_NAND_TypeDef *device;
	  device = FSMC_Bank2_3;
  uint32_t eccval = 0x00000000;

    /* Get the ECCR2 register value */
    eccval = device->ECCR2;


  /* Return the error correction code value */
  return(eccval);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
