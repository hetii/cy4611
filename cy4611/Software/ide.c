//-----------------------------------------------------------------------------
//  Copyright (c) 1999 Cypress Semiconductor, Inc. All rights reserved
//-----------------------------------------------------------------------------
//
// This file contains the IDE specific portions of the code.  In ATAPI
// or SCSI applications, this file should not be needed.
//
// $Archive: /USB/atapifx2/software/ide.c $
// $Date: 1/23/02 9:35a $
// $Revision: 53 $
//-----------------------------------------------------------------------------
#include "Fx2.h"
#include "Fx2regs.h"
#include "gpif.h"
#include "scsi.h"

static bit ideReadCommand(bit verify);
static bit ideWriteCommand();
bit ideUdmaWriteCommand();
static void IDEnop();
static bit checkForMedia();
static DWORD dwLBA;     // This is global to help the optimizer
void dwLBAtoLBARegs();      // Stuff the LBA registers


// From SCSI spec SPC (SCSI primary commands)
// Byte 0 -- 70 = Current error
// Byte 1 -- Segment number
// Byte 2 -- Sense key
// Byte 3-6 -- Information (not used)
// Byte 7 -- add'l sense length 
// byte 8-11 -- Command specific information
// byte 12 -- ASC (Add'l sense code)
// byte 13 -- ASQ (Add'l sense qualifier)
//                                                       Key                                                         ASC   ASQ
//                                             0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
const char code senseCRCError[] =          {0x70, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00};   
const char code senseInvalidFieldInCDB[] = {0x70, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};   
const char code senseOk[] =                {0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};   
char code * sensePtr;

extern char NumCylindersMSB; // global #Cyl MSB
extern char NumCylindersLSB; // global #Cyl LSB
extern char NumHeads;        // global #Heads
extern char NumSectPerTrack; // global #SectorsPerTrack

/////////////////////////////////////////////////////////////////////////////////
#if DEVICE_TYPE_IS_IDE
/////////////////////////////////////////////////////////////////////////////////


bit generalIDEInCommand()
{
   BYTE cmd;
   WORD packetLen;
   bit status;
   
   cmd = EP2FIFOBUF[0xf];  
   
   switch (cmd)
   {
      // Minimum processing for a case in this switch statement:
      case INQUIRY:
      {
         sensePtr = senseOk;
         packetLen = min(dataTransferLen, (WORD) sizeof(SCSIInquiryData));
         
         // relinquish control of the bulk buffer occupied by the CBW
         EP2BCL = 0x80;     

         // Send out our stored inquiry data
         waitForInBuffer();
         mymemmovexx(EP8FIFOBUF, SCSIInquiryData_, packetLen);   // NOTE: may not be needed anymore since FIFO in AUTOIN 
         
         if (packetLen)
         {
            EP8BCH = MSB(packetLen);
            EP8BCL = LSB(packetLen);
         
            dataTransferLen -= packetLen;
         }
         
         return(USBS_PASSED);
      }
            
      case READ_10:
      {
         sensePtr = senseOk;
         checkForMedia();

         if (sensePtr == senseOk)
         {
            return(ideReadCommand(0));   
         }
         else
         {
            // relinquish control of the bulk buffer occupied by the CBW
            EP2BCL = 0x80;     
            
            failedIn();         
            return(USBS_FAILED);
         }
      }
                
      case VERIFY_10:
      {
         sensePtr = senseOk;
         checkForMedia();
         if (sensePtr == senseOk)
            return(ideReadCommand(1));   // ideReadCommand sets the OUT2BC = 0
         else
         {
            // relinquish control of the bulk buffer occupied by the CBW
            EP2BCL = 0x80;     
            
            failedIn();         
            return(USBS_FAILED);
         }
      }            
      case SEEK_10:
      {
         sensePtr = senseOk;
         checkForMedia();

         // relinquish control of the bulk buffer occupied by the CBW
         EP2BCL = 0x80;     
            
         if (sensePtr == senseOk)
         {
            return(USBS_PASSED);
         }
         else
         {
            failedIn();         
            return(USBS_FAILED);
         }
      }                

      case READ_FORMAT_CAPACITIES:
      case READ_CAPACITY:
      {
         BYTE cap_hdr_offset = 0;
         BYTE num_dwords = 2;

         // relinquish control of the bulk buffer occupied by the CBW
         EP2BCL = 0x80;     
         
         sensePtr = senseOk;
         checkForMedia();
                 
         waitForInBuffer();
         if (sensePtr == senseOk)
         {
            if(cmd == READ_FORMAT_CAPACITIES) // add 4 byte capacity list header
            {
               EP8FIFOBUF[0] = 0x0;
               EP8FIFOBUF[1] = 0x0;
               EP8FIFOBUF[2] = 0x0;
               EP8FIFOBUF[3] = 0x08;
               cap_hdr_offset = 4;
               num_dwords = 3;
            }
            EP8FIFOBUF[0+cap_hdr_offset] = ((BYTE *) &driveCapacity)[0];
            EP8FIFOBUF[1+cap_hdr_offset] = ((BYTE *) &driveCapacity)[1];
            EP8FIFOBUF[2+cap_hdr_offset] = ((BYTE *) &driveCapacity)[2];
            EP8FIFOBUF[3+cap_hdr_offset] = ((BYTE *) &driveCapacity)[3];
            EP8FIFOBUF[4+cap_hdr_offset] = (ATA_SECTOR_SIZE >> 24) & 0xff;
            EP8FIFOBUF[5+cap_hdr_offset] = (ATA_SECTOR_SIZE >> 16) & 0xff;
            EP8FIFOBUF[7+cap_hdr_offset] = (ATA_SECTOR_SIZE >>  0) & 0xff;
            EP8FIFOBUF[6+cap_hdr_offset] = (ATA_SECTOR_SIZE >>  8) & 0xff;
            
            packetLen = min(sizeof(DWORD) * num_dwords, dataTransferLen);
            if (packetLen)
            {
               EP8BCH = MSB(packetLen);
               EP8BCL = LSB(packetLen);
               dataTransferLen -= packetLen;
            }
            status = USBS_PASSED;
         }
         else
         {
            failedIn();
            status = USBS_FAILED;
         }

         return(status);
      }

      case TEST_UNIT_READY:
      case PREVENT_ALLOW_MEDIUM_REMOVAL:
      {
      // relinquish control of the bulk buffer occupied by the CBW
         EP2BCL = 0x80;     
         
         if (dataTransferLen)    // This command shouldn't have any data!
            failedIn();         
         
         checkForMedia();
         
         if (sensePtr == senseOk)
            return(USBS_PASSED);
         else
            return(USBS_FAILED);
      }                
      case REQUEST_SENSE:
      {
         // relinquish control of the bulk buffer occupied by the CBW
         EP2BCL = 0x80;     
   
         waitForInBuffer();

         mymemmovexx(EP8FIFOBUF, (char xdata *) sensePtr, sizeof(senseOk));

         packetLen = min(sizeof(senseOk), dataTransferLen);   
         if (packetLen)
         {
            EP8BCH = MSB(packetLen);
            EP8BCL = LSB(packetLen);

            dataTransferLen -= packetLen;
         }
         sensePtr = senseOk;
         return(USBS_PASSED);
      }

      case MODE_SENSE_10:
      {               
         BYTE pagenum;
         
         pagenum = EP2FIFOBUF[CBW_DATA_START+2] & 0x3F; // identify page (see p.141 SCSI 2ed.)
   
         EP2BCL = 0x80; // relinquish control of the bulk buffer occupied by the CBW
         
         waitForInBuffer();
         
         if((pagenum != 0x05) && (pagenum != 0x3F)
             && (pagenum != 0x01) && (pagenum != 0x08) && (pagenum != 0x1B))
         { // only respond to requests for certain pages (the mandatory ones plus page 5)
            sensePtr = senseInvalidFieldInCDB;
            failedIn();
            return(USBS_FAILED);
         }
   
         // If one of the supported pages is requested, return the 8 byte Mode Parameter Header
         // plus a single 12 byte page. Only the Mode Data length (LSB) is significant in the
         // Mode Parameter Header. It has a Vendor Specific Medium Type Code in byte 2, and
         // a 1 bit WP Write Protect bit in byte 3 that are not initialized here.
         // Pages 0x01, and 0x08 do not have significant data - they are spoofed.
         // Page 0x1B has a TLUN field in byte 3 that is initialized to 1 (as per ISD).
         // Page 0x05 does contain information that is needed to boot to HDD and CDROM.
         // Page 0x3F, All Pages, is also responded to, I just return the single page 5 though.
         // I do this because page 5 is the only page with significant information anyway, and
         // this simplification reduces the code by eliminating the need to return a number of
         // additional pages (which would change if additional pages are later supported).
         // The supported pages are (see INF-8070_1_3.pdf p37 Table 25):
         //   case 0x01:                          // Read-Write Error Recovery Page
         //   case 0x08:                          // Caching Page
         //   case 0x05:                          // Flexible Disk Page: needed to boot from USB
         //   case 0x1B:                          // Removable Block Access Capabilities Page
         //   case 0x3F:                          // All Pages
         
         EP8FIFOBUF[0] = 0x00;
         mymemmovexx(EP8FIFOBUF+1, EP8FIFOBUF, 18-1); // clear buffer - 18 bytes in all responses
         
         EP8FIFOBUF[1] = 0x12; // Mode Data length (LSB) in Mode Parameter Header is 
                               // 8(MPH) + 12(1 page) - 2(offset of 2 byte length field itself)           
         
         EP8FIFOBUF[8+0] = pagenum;         // fill out the page num - fields are all 0x0
         EP8FIFOBUF[8+1] = 0x0A;            // set individual Page Length
   
         if((pagenum == 0x05) || (pagenum == 0x3F))
         {  // Note: a request for All Pages just returns the single page 5 here
            EP8FIFOBUF[8+0] = 0x05;            
            if(EZUSB_HIGHSPEED())
            {
               EP8FIFOBUF[8+2] = 0xFF;         // HS Transfer Rate (MSB) (field limited to 65Mb/Sec)
               EP8FIFOBUF[8+3] = 0xFF;         // HS Transfer Rate (LSB)
            }
            else
            {
               EP8FIFOBUF[8+2] = 0x2E;         // FS Transfer Rate (MSB) (12Mb/Sec) 
               EP8FIFOBUF[8+3] = 0xE0;         // FS Transfer Rate (LSB)
            }
            EP8FIFOBUF[8+4] = NumHeads;        // #Heads
            EP8FIFOBUF[8+5] = NumSectPerTrack; // #SectorsPerTrack
            EP8FIFOBUF[8+6] = (ATA_SECTOR_SIZE >>  8) & 0xff; // Data Bytes per sector (truncated)
            EP8FIFOBUF[8+7] = (ATA_SECTOR_SIZE >>  0) & 0xff; // Data Bytes per sector
            EP8FIFOBUF[8+8] = NumCylindersMSB; // #Cyl MSB
            EP8FIFOBUF[8+9] = NumCylindersLSB; // #Cyl LSB
         }
         else if(pagenum == 0x1B)
         {
            EP8FIFOBUF[8+3] = 0x01;            // set TLUN = 1 for page 0x1B
         }
   
         packetLen = min(0x12, dataTransferLen);
         if (packetLen)
         {
            EP8BCH = MSB(packetLen);
            EP8BCL = LSB(packetLen);
         
            dataTransferLen -= packetLen;
         }
         sensePtr = senseOk;
         return(USBS_PASSED);
      } // end case

      case MODE_SELECT_06:
      case MODE_SENSE_06:
      case STOP_START_UNIT:
      default:
      {
         // relinquish control of the bulk buffer occupied by the CBW
         EP2BCL = 0x80;     
         
         sensePtr = senseInvalidFieldInCDB;
         failedIn();
         return(USBS_FAILED);
      }
   }
}   

bit generalIDEOutCommand()
{
   BYTE cmd;
   
   cmd = EP2FIFOBUF[0xf];  
   
   switch (cmd)
   {
      case WRITE_10:
         sensePtr = senseOk;
         checkForMedia();
         if (sensePtr == senseOk)
            return(ideWriteCommand());
         else
         {
            // relinquish control of the bulk buffer occupied by the CBW
            EP2BCL = 0x80;
            if (dataTransferLen)
               stallEP2OUT();

            return(USBS_FAILED);
         }
                
      default:
         // relinquish control of the bulk buffer occupied by the CBW
         EP2BCL = 0x80;     
         
         if (dataTransferLen)
            stallEP2OUT();
         
         return(USBS_FAILED);
         break;
   }
}   



void waitForInBuffer()
{
   while((EP8CS & bmEPFULL));   // Wait for an available buffer from the host

   return;
}   


static bit ideReadCommand(bit verify)
{   
   BYTE driveStatus;
   WORD sectorcount; 
   BYTE i; 
   WORD wordCtr;

   writePIO8(ATA_DRIVESEL_REG, 0xe0);
   if (waitForBusyBit() == USBS_FAILED)
   {
      // Oddly enough, an error bit is okay here.  It means that the LAST command failed, not this one.
      // A new command is required to clear many error conditions.
   }
        
   ((BYTE *) &dwLBA)[0] = EP2FIFOBUF[CBW_DATA_START+2];
   ((BYTE *) &dwLBA)[1] = EP2FIFOBUF[CBW_DATA_START+3];
   ((BYTE *) &dwLBA)[2] = EP2FIFOBUF[CBW_DATA_START+4];
   ((BYTE *) &dwLBA)[3] = EP2FIFOBUF[CBW_DATA_START+5];

   // relinquish control of the bulk buffer occupied by the CBW
   EP2BCL = 0x80;  

   // This loop breaks up the 32 bit length into 8 bits * sectors.
   // For example, a read of 0x10000 turns into 0x80 sectors.
   while (dataTransferLen)
   {
      dwLBAtoLBARegs();

      // First stuff the length register (number of sectors to read)
      if (dataTransferLenMSW & 0xfffe)
      {
         writePIO8(ATA_SECTOR_COUNT_REG, 1);     // if (bExtAddrSupport) we need to stuff the MSB
         writePIO8(ATA_SECTOR_COUNT_REG, 0);     // 0 means 256 blocks of 512 bytes -- Max drive xfer, max TC
         sectorcount = 0x100;
      }
      else
      {
         sectorcount = (dataTransferLenLSW + ATA_SECTOR_SIZE-1)/ATA_SECTOR_SIZE + (dataTransferLenMSW & 1) * 0x80;
         writePIO8(ATA_SECTOR_COUNT_REG, 0);      // for extended addressing
         writePIO8(ATA_SECTOR_COUNT_REG, sectorcount);       // divide len into blocks
      }
        
      dwLBA += sectorcount;
      
      if (!udmaMode || verify || (dataTransferLenLSW & 0x1ff))    // UDMA cannot handle sub-sector sized reads
      {
         // Execute the read command
         if (bExtAddrSupport)
            {
            if (verify)
               writePIO8(ATA_COMMAND_REG, ATA_COMMAND_VERIFY_10_EXT);
            else
               writePIO8(ATA_COMMAND_REG, ATA_COMMAND_READ_10_EXT);
            }
         else
            {
            if (verify)
               writePIO8(ATA_COMMAND_REG, ATA_COMMAND_VERIFY_10);
            else
               writePIO8(ATA_COMMAND_REG, ATA_COMMAND_READ_10);
            }

           
         // The verify command reads from the drive, but doesn't transfer data
         // to us.
         if (verify)     
         {
            if(waitForBusyBit() == USBS_FAILED)
               return(USBS_FAILED);
            else
               continue;
         }
   
         // set up for GPIF transfer - wordwide
         //EP8GPIFTCH = MSB(wPacketSize >> 1);     
         //EP8GPIFTCL = LSB(wPacketSize >> 1);
         
         while (sectorcount--)
         {
            // Wait for the drive to be non-busy and have either data or an error
            while (1)
            {
               driveStatus = readATAPI_STATUS_REG();                             
               if ((driveStatus & (ATAPI_STATUS_BUSY_BIT | ATAPI_STATUS_DRQ_BIT)) == ATAPI_STATUS_DRQ_BIT)
                  break;
               if (driveStatus & (ATAPI_STATUS_BUSY_BIT | ATAPI_STATUS_ERROR_BIT) == ATAPI_STATUS_ERROR_BIT)
                  break;
            }

            // If there's an error, the drive may still want to send us the data.
            if (driveStatus & ATAPI_STATUS_ERROR_BIT)
               {
               readPIO8(ATAPI_ERROR_REG);
               if (driveStatus & ATAPI_STATUS_DRQ_BIT)
                  for (wordCtr = 0; wordCtr < 0x100; wordCtr++)
                     readPIO8(ATAPI_DATA_REG);
               failedIn();
               return(USBS_FAILED);  
               }
            else
            {
               BYTE bLimit;
               WORD wThisPacketSize;

               if (wPacketSize == 0x40)
                  bLimit = 8;
               else 
                  bLimit = 1;
               for (i = 0; i < bLimit && dataTransferLen; i++)
               {
                  waitForInBuffer();

                  wThisPacketSize = min(wPacketSize, dataTransferLen);
                  readPIO16(ATAPI_DATA_REG, wThisPacketSize+1);
                  while (!gpifIdle())     // Wait for xfer to complete
                     ;
                  EP8BCH = MSB(wThisPacketSize);
                  EP8BCL = LSB(wThisPacketSize);
                  dataTransferLen -= wThisPacketSize; 
               }
            }
         }//while (sectorcount--)
      }
      else  // transfer is udma mode
      {
         initUdmaRead();
         if (bExtAddrSupport)
            writePIO8(ATAPI_COMMAND_REG, ATA_COMMAND_DMAREAD_RETRY_EXT);
         else
            writePIO8(ATAPI_COMMAND_REG, ATA_COMMAND_DMAREAD_RETRY);
         readUDMA((DWORD) sectorcount << 8);     // Words = sectors * 256
         driveStatus = readATAPI_STATUS_REG();
         if (driveStatus & ATAPI_STATUS_ERROR_BIT) 
            {
            if (readPIO8(ATAPI_ERROR_REG) & ATAPI_ERROR_ICRC_BIT)
               {
               sensePtr = senseCRCError;
               }
            // No need to do failedIn() -- All data is already xferred.
            return(USBS_FAILED);
            }

         // Two choices -- Either we're limited to 0x100 sectors, or limited to dataTransferLen.
         // BUGBUG -- No capability here to report Hi > Di.
         if (sectorcount == 0x100)
            dataTransferLenMSW -= 2;
         else
            dataTransferLen = 0;
      }
   }//while (dataTransferLen)
   
   return(USBS_PASSED);
}   

static bit ideWriteCommand()
{
   WORD savebc;
   BYTE driveStatus;
   WORD sectorcount; 
   BYTE i; 
    
   writePIO8(ATA_DRIVESEL_REG, 0xe0);
   if (waitForBusyBit() == USBS_FAILED)
   {
      // Oddly enough, an error bit is okay here.  It means that the LAST command failed, not this one.
      // A new command is required to clear many error conditions.
   }
    
   ((BYTE *) &dwLBA)[0] = EP2FIFOBUF[CBW_DATA_START+2];
   ((BYTE *) &dwLBA)[1] = EP2FIFOBUF[CBW_DATA_START+3];
   ((BYTE *) &dwLBA)[2] = EP2FIFOBUF[CBW_DATA_START+4];
   ((BYTE *) &dwLBA)[3] = EP2FIFOBUF[CBW_DATA_START+5];

   // relinquish control of the bulk buffer occupied by the CBW
   EP2BCL = 0x80;     
  
   // If there's no data length, just exit once we've freed the CBW buffer
   if (!dataTransferLen)
      return USBS_PASSED;

   // Send the command to the drive
   // This loop breaks up the 32 bit length into 8 bits * sectors.
   // For example, a read of 0x10000 turns into 0x80 sectors.
   while (dataTransferLen)
   {
      dwLBAtoLBARegs();      // Stuff the LBA registers
      
      // First stuff the length register (number of sectors to write)
      if (dataTransferLenMSW & 0xfffe)
      {
         writePIO8(ATA_SECTOR_COUNT_REG, 0x1);      // 1 for ext addr support 
         writePIO8(ATA_SECTOR_COUNT_REG, 0x0);      // 0 means 256 blocks of 512
         sectorcount = 0x100;
      }
      else
      {
         sectorcount = (dataTransferLenLSW+ATA_SECTOR_SIZE-1)/ATA_SECTOR_SIZE 
            + (dataTransferLenMSW & 1) * 0x80;
         writePIO8(ATA_SECTOR_COUNT_REG, 0);      // 0 for MSB of ext address
         writePIO8(ATA_SECTOR_COUNT_REG, sectorcount);       // divide len into blocks
      }
        
      dwLBA += sectorcount;

      if (udmaMode && !(dataTransferLenLSW & 0x1ff))
      {
         // Execute the write command
         if (bExtAddrSupport)
            writePIO8(ATA_COMMAND_REG, ATA_COMMAND_DMAWRITE_RETRY_EXT);
         else
            writePIO8(ATA_COMMAND_REG, ATA_COMMAND_DMAWRITE_RETRY);
         
         if (sectorcount == 0x100)
            writeUDMA((DWORD)0x10000);     // 64k Transfers = 128K
         else
            writeUDMA((dataTransferLen+1) >> 1);
         
         // Check status to clear interrupt.
         driveStatus = readATAPI_STATUS_REG();
         
         if (driveStatus & ATAPI_STATUS_ERROR_BIT) 
         {
            driveStatus = readPIO8(ATAPI_ERROR_REG);
            if (driveStatus & ATAPI_ERROR_ICRC_BIT)
            {
               sensePtr = senseCRCError;
            }
            return(USBS_FAILED);
         }
         else
         {
            if (sectorcount == 0x100)
               dataTransferLenMSW -= 2;
            else
               dataTransferLen = 0;
         }
      }
      else
      {
         // Execute the write command
         if (bExtAddrSupport)
            writePIO8(ATA_COMMAND_REG, ATA_COMMAND_WRITE_10_EXT);
         else
            writePIO8(ATA_COMMAND_REG, ATA_COMMAND_WRITE_10);
         
         while (sectorcount--)
         {
            BYTE limit;
   
            // Wait for the drive to be non-busy and have either data or an error
            while (1)
            {
               driveStatus = readATAPI_STATUS_REG();                             
               if ((driveStatus & (ATAPI_STATUS_BUSY_BIT | ATAPI_STATUS_DRQ_BIT)) == ATAPI_STATUS_DRQ_BIT)
                  break;
               if (driveStatus & (ATAPI_STATUS_BUSY_BIT | ATAPI_STATUS_ERROR_BIT) == ATAPI_STATUS_ERROR_BIT)
                  break;
            }
               
            // Normal case -- Got a sector.  Send it to the drive (in 8 chunks)
            if (wPacketSize == 0x40)
               limit = 8;
            else
               limit = 1;
   
            for (i = 0; i < limit; i++)
            {
               while(EP2CS & bmEPEMPTY);       // Wait for host to send data
               savebc = (EP2BCH << 8) | EP2BCL;
                   
               // Terminate xfer on receipt of short packet, otherwise drop
               // into streamlined case
               if (savebc < wPacketSize)
               {
                  EP2BCL = 0;
                  writePIO16(ATAPI_DATA_REG, savebc+1);     // Add 1 to allow odd xfers.
                  
                  dataTransferLen -= savebc + i * wPacketSize;
                  goto stopOnShortPacket;
               }
               else
               {
                  EP2BCL = 0;
                  writePIO16(ATAPI_DATA_REG, wPacketSize);
               }
            }
            dataTransferLen -= ATA_SECTOR_SIZE; // Returned a full sector.
         }  // while (sectorcount) 
      } // else (if udma)
   } // While (dataTransferLen)
    
stopOnShortPacket:

   if (driveStatus & ATAPI_STATUS_ERROR_BIT)
      return(USBS_FAILED);
   else
      return(USBS_PASSED);
}


// Don't have the ability to sense media (yet)
static bit checkForMedia()
{
   return(1);
}

void dwLBAtoLBARegs()      // Stuff the LBA registers
{
   writePIO8(ATA_DRIVESEL_REG, 0xe0);

   if (bExtAddrSupport)
      {
      writePIO8(ATA_LBA_LSB_REG,  ((BYTE *) &dwLBA)[0]);    // LBA (31:24)
      writePIO8(ATA_LBA_2SB_REG,  0);                       // LBA (39:32)
      writePIO8(ATA_LBA_MSB_REG,  0);                       // LBA (47:40)
      }

   writePIO8(ATA_LBA_LSB_REG,  ((BYTE *) &dwLBA)[3]);    // LBA (7:0)
   writePIO8(ATA_LBA_2SB_REG,  ((BYTE *) &dwLBA)[2]);    // LBA (15:8)
   writePIO8(ATA_LBA_MSB_REG,  ((BYTE *) &dwLBA)[1]);    // LBA (23:16)

   if (!bExtAddrSupport)
      {      
      writePIO8(ATA_DRIVESEL_REG, ((BYTE *) &dwLBA)[0] | 0xe0);
      }
}



/////////////////////////////////////////////////////////////////////////////////
#endif      // DEVICE_TYPE_IS_IDE
/////////////////////////////////////////////////////////////////////////////////

