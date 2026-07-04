//-----------------------------------------------------------------------------
//  Copyright (c) 1999-2001 Cypress Semiconductor, Inc. All rights reserved
//-----------------------------------------------------------------------------
//
// This file contains the device initialization code.  
//
// $Archive: /USB/atapifx2/software/atareset.c $
// $Date: 1/23/02 9:37a $
// $Revision: 44 $
//-----------------------------------------------------------------------------
#include "fx2.h"
#include "fx2regs.h"
#include "gpif.h"

char NumCylindersMSB; // global #Cyl MSB
char NumCylindersLSB; // global #Cyl LSB
char NumHeads;        // global #Heads
char NumSectPerTrack; // global #SectorsPerTrack

char MaxPIO;
char udmaMode;      // global to keep track of which udma mode we are in
xdata BYTE halfKBuffer[BUFFER_SIZE];
idata char localSerialNumber[ATAPI_INQUIRY_SERIAL_LEN*2];
bit bExtAddrSupport;

void resetATAPIDevice()
{
   // Select ATAPI device, read it back
   bit done = 0;
   BYTE driveStatus;
   #define MAX_COUNT 200

   BYTE count;

   while (!done)
   {
      // Pound the reset line
      hardwareReset();

      EZUSB_Delay(900);            // Mitsumi CR-4808TE(CYSD007) is a good test for this number.

      // Wait for the completion
      for (count = 0, driveStatus = readATAPI_STATUS_REG(); 
           count < MAX_COUNT 
               && ((driveStatus & ATAPI_STATUS_BUSY_BIT) || (driveStatus == 0x7f)) ; 
           driveStatus = readATAPI_STATUS_REG(), count++)
      {
         EZUSB_Delay(50);            // Wait 50 ms to be polite
      }
        
      if (count == MAX_COUNT)  
         continue;

      // Byte count should be a special code -- 0xeb14
      if (readPIO8(ATAPI_BYTE_COUNT_MSB) == 0xeb && readPIO8(ATAPI_BYTE_COUNT_LSB) == 0x14)
      {
         done = 1;
         scsi = 1;
      }
      else
      // device = ide
      {
         writePIO8(ATAPI_NULL_REG, 0xff);        // Pre-load the bus with 0xff -- Make sure someone drives the 00's
         if ((readPIO8(ATA_ERROR_REG) & 0x7f) == 1)
         {
            done = 1;
            scsi = 0;
         }
         else
            EZUSB_Delay(500);           // Wait 500 ms to make sure it's really ready.
      }

   }

   EZUSB_Delay(500);           // Wait 500 ms to make sure it's really ready.

   #if DEVICE_TYPE_IS_SCSI
   if (scsi)
      SCSITestUnitReady();
   #endif
}



void ATAPIIdDevice()
{
   BYTE i;
   BYTE driveStatus;

   // Wait for the register block to be non-busy and for a "drive ready" condition
   do
   {
      writePIO8(ATAPI_DRIVESEL_REG, 0xa0);
      driveStatus = readATAPI_STATUS_REG();
   } while((driveStatus & (ATAPI_STATUS_BUSY_BIT | ATAPI_STATUS_DRDY_BIT)) != (ATAPI_STATUS_DRDY_BIT));    // Do-while

   writePIO8(ATA_ERROR_REG, 0);            // Copied from Phoenix BIOS
   writePIO8(ATA_SECTOR_COUNT_REG, 1);
   writePIO8(ATA_LBA_LSB_REG     , 0xff);
   writePIO8(ATA_LBA_2SB_REG     , 0xff);
   writePIO8(ATA_LBA_MSB_REG     , 0);
   writePIO8(ATAPI_DRIVESEL_REG, 0xa0);
   writePIO8(ATAPI_CONTROL_REG, 0);        // Added later -- Make sure the nIEN bit is clear (active)


   // Send Identify device command
   if (scsi)
      writePIO8(ATAPI_COMMAND_REG, ATAPI_COMMAND_ID_DEVICE);
   else
      writePIO8(ATAPI_COMMAND_REG, IDE_COMMAND_ID_DEVICE);

   waitForIntrq();
        
   // Wait for the register block to be non-busy and to have data ready
   do
   {
      driveStatus = readATAPI_STATUS_REG();
      if (driveStatus & ATAPI_STATUS_ERROR_BIT) // don't get stuck here with an error
         return;     
   } while((driveStatus & (ATAPI_STATUS_BUSY_BIT | ATAPI_STATUS_DRQ_BIT)) != (ATAPI_STATUS_DRQ_BIT));    // Do-while

   // Read the data from the drive
   {
      char timeout = 10;
      
      while (!readPIO16toXdata(ATAPI_DATA_REG, halfKBuffer, BUFFER_SIZE, IGNORE_DRIVE_LEN))
         if (!timeout--)
            break;
   }

   // Place the fields that we need into the SCSI block
   for (i = 0; i < SCSI_INQUIRY_MANUFACTURER_LEN; i++)
   {
      // swap bytes within words.  This is stored backwards!
      SCSIInquiryData_[SCSI_INQUIRY_MANUFACTURER+i] = halfKBuffer[ATAPI_INQUIRY_MANUFACTURER * 2 + (i ^ 1)];
   }

   SCSIInquiryData_[SCSI_INQUIRY_REVISION+1] = halfKBuffer[ATAPI_INQUIRY_REVISION * 2]+'0';
   SCSIInquiryData_[SCSI_INQUIRY_REVISION+3] = halfKBuffer[ATAPI_INQUIRY_REVISION * 2 +2]+'0';
   SCSIInquiryData_[SCSI_INQUIRY_REMOVABLE_BYTE] |= SCSI_INQUIRY_REMOVABLE_BIT & halfKBuffer[ATAPI_INQUIRY_REMOVABLE_BYTE];
        
   // BUG -- what is this doing? Reduce the command set for ATA
   if (!scsi)
      SCSIInquiryData_[SCSI_INQUIRY_DATA_FORMAT] = 0;
        
   // Copy serial number to our local storage area, converting zeroes to text.
   // It's stored as double byte characters, so zero every other byte.
   for (i = 0; i < ATAPI_INQUIRY_SERIAL_LEN; i++)
   {
      if (halfKBuffer[i+ATAPI_INQUIRY_SERIAL])
         localSerialNumber[i+i] = halfKBuffer[i+ATAPI_INQUIRY_SERIAL];
      else
         localSerialNumber[i+i] = '0';
      localSerialNumber[i+i+1] = 0;
   }

   // Check for large disk (48 bit) support.  ATA-6 spec below....
   // 6.2.1
   //    4) The contents of words (61:60) and (103:100) shall not be used to determine if 48-bit addressing is
   //       supported. IDENTIFY DEVICE bit 10 word 83 indicates support for 48-bit addressing.
   if (halfKBuffer[IDENTIFY_48BIT_ADDRESSING+1] & (1<<2))
      {
      bExtAddrSupport = 1;
      // This is actually smaller than a loop of 4!!
      // Yes, this only supports 0x100 00 00 00 sectors, which is 220,000GB (industry GB, not true)
      ((BYTE *)&driveCapacity)[3] = halfKBuffer[0+IDE_ID_TOTAL_48_BIT_SECTORS_LSW];
      ((BYTE *)&driveCapacity)[2] = halfKBuffer[1+IDE_ID_TOTAL_48_BIT_SECTORS_LSW];
      ((BYTE *)&driveCapacity)[1] = halfKBuffer[2+IDE_ID_TOTAL_48_BIT_SECTORS_LSW];
      ((BYTE *)&driveCapacity)[0] = halfKBuffer[3+IDE_ID_TOTAL_48_BIT_SECTORS_LSW];
      }
   else
      {
      bExtAddrSupport = 0;
      // This is actually smaller than a loop of 4!!
      ((BYTE *)&driveCapacity)[3] = halfKBuffer[0+IDE_ID_TOTAL_SECTORS_LSW];
      ((BYTE *)&driveCapacity)[2] = halfKBuffer[1+IDE_ID_TOTAL_SECTORS_LSW];
      ((BYTE *)&driveCapacity)[1] = halfKBuffer[2+IDE_ID_TOTAL_SECTORS_LSW];
      ((BYTE *)&driveCapacity)[0] = halfKBuffer[3+IDE_ID_TOTAL_SECTORS_LSW];
      }

   NumCylindersMSB = halfKBuffer[IDENTIFY_NUM_CYLINDERS_MSB];
   NumCylindersLSB = halfKBuffer[IDENTIFY_NUM_CYLINDERS_LSB];
   NumHeads = halfKBuffer[IDENTIFY_NUM_HEADS];
   NumSectPerTrack = halfKBuffer[IDENTIFY_NUM_SECT_PER_TRACK];

   // check for PIO3 support (or greater)
   // 64 = word index to ID Device parameter block - LSB=value, MSB=reserved
   // if( (!scsi) && ( MaxPIO = halfKBuffer[(64*2)]) ).  WaveDataPio4 is used
   // for both PIO3 and PIO4, with PIO3 @ 30MHz, and PIO4 @ 48Mhz
   MaxPIO = halfKBuffer[IDENTIFY_ADVANCED_PIO];
   udmaMode = 0;

   // Check for UDMA support
   if ((halfKBuffer[IDENTIFY_FIELD_VALIDITY] & bmBIT2) &&
       (halfKBuffer[IDENTIFY_UDMA_MODES] & (UDMA_MODE2 | UDMA_MODE4)))
   {
      if (halfKBuffer[IDENTIFY_UDMA_MODES] & UDMA_MODE4)
      {
         udmaMode = TRANSFER_MODE_UDMA4;
      }
      else if (halfKBuffer[IDENTIFY_UDMA_MODES] & UDMA_MODE2)
      {
         udmaMode = TRANSFER_MODE_UDMA2;
      }
   }

   // If UDMA is supported, enable it.  If not, enable the highest PIO mode
   if (udmaMode)
   {
      configureATATransferMode(udmaMode);
      udmaErrorCount = 0;
      mymemmovexx(&GPIF_WAVE_DATA, (BYTE xdata *) WaveDataPio4, sizeof(WaveDataPio4));
   }
   else if(MaxPIO)
   {
      if(MaxPIO & PIO4) 
      {
         IFCONFIG |= 0x40;       // SET CLOCK TO 48MHZ
         configureATATransferMode(PIO_MODE4);                  // SCR_PIO4=0x0C, PIO-mode4
      }
      else if(MaxPIO & PIO3) 
      {
         configureATATransferMode(PIO_MODE3);                  // SCR_PIO3=0x0B, PIO-mode3
      }

      mymemmovexx(&GPIF_WAVE_DATA, (BYTE xdata *) WaveDataPio4, sizeof(WaveDataPio4));
   }


   driveCapacity -= 1;  // The command that reads drive capacity actually wants the last valid LBA.
   return;
}

void configureATATransferMode(BYTE mode)
{
      // select the drive and set new speed
      writePIO8(ATAPI_DRIVESEL_REG, 0xa0);
      writePIO8(ATA_SECTOR_COUNT_REG, mode);      
      writePIO8(ATAPI_FEATURE_REG, 0x03);                            // opcode 0x03 used for transfer mode
      writePIO8(ATAPI_COMMAND_REG, ATAPI_COMMAND_SET_FEATURES);      // execute the command   
}
