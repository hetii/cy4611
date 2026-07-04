//-----------------------------------------------------------------------------
//   File:      scsi.c
//   Contents:   Functions to handle a scsi device.
//
// indent 3.  NO TABS!
//
// Description:
//    SCSI devices differ from IDE device as follows:
//    - SCSI devices may accept or return data according to the value of
//       its byte count registers which may differ from the host request
//       indicated in the CBW variable 'dataTransferlen'. 
//
//       If the host requests to write a greater amount than the byte count 
//       registers indicate, the extra data is processed by throwing it on 
//       the floor and reporting it as a residue.
//
//       If the host requests a read greater than what is indicated in the 
//       byte count registers, and the last packet is a full packet, a short 
//       packet is sent to terminate the transfer.
//
//    - generalSCSIInCommand() and generalSCSIOutCommand() are called from
//       periph.c.  Their conditional compile flags are kept inside the function
//       to maintain original code for periph.c.  All other functions in this
//       module are specific for SCSI and are static to this file.  They are
//       under one conditional compile flag.
//
//-----------------------------------------------------------------------------
// $Archive: /USB/atapifx2/software/scsi.c $
// $Date: 1/21/02 2:38p $
// $Revision: 41 $
//
//-----------------------------------------------------------------------------
//  Copyright (c) 1999 Cypress Semiconductor, Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "fx2.h"
#include "fx2regs.h"
#include "scsi.h"
#include "gpif.h"

static BYTE scsiWrite(void);
static BYTE scsiWriteUdma();
static void prepareForATAPICommand();
static bit inDataFromDriveUdma();


// this bit determines if the current transfer is to be carried out using UDMA (1) or
// PIO (0)
bit useUdma;
bit bShortPacketSent;
BYTE udmaErrorCount;

#define SENSE_LEN 18

//-----------------------------------------------------------------------------
// Function:  generalSCSIInCommand()
//
// Input:   none
// Output:  bit flag
//          0 = success, 1 = failure
//
// Global data:
//    CBW from EP2FIFOBUF.
//
// Description:
//    Top level handler for scsi read.  The scsi command packet is 
//    a 12-byte packet extracted from the CBW contained in EP2FIFOBUF 
//    starting from byte 15.  If the command fails, the IN endpoint buffer
//    is stalled and the transaction is failed.
//
//    If the command was processed successfully, data is extracted from the 
//    drive till the byte count from the drive is exhausted.  If the byte count
//    indicated by the drive is less than what is requested by the host, the IN
//    endpoint is stalled, but the transaction is passed.  The remainder of bytes
//    the host still expects is reported as a residue.
//-----------------------------------------------------------------------------
BYTE generalSCSIInCommand()
{
#if DEVICE_TYPE_IS_SCSI

   BYTE result = 0;

   // Clear the bit telling us if we need a STALL to terminat the IRP on the host side   
   bShortPacketSent = 0;

   // if the drive is configured for udma then use udma depending on the
   // scsi command
   if (udmaMode)
   {
      switch(EP2FIFOBUF[0xf])
      {
         case 0x28:
         case 0xA8:
            useUdma = 1;     
            break;
         default:
            useUdma = 0;
            break;
      }
   }

   result = sendSCSICommand(EP2FIFOBUF + CBW_DATA_START);

   // relinquish control of the bulk buffer occupied by the CBW
   EP2BCL = 0x80; 
   
   // Need to modify this code so that we know if we sent a short packet to terminate the xfer.
   // Although the STALL is required, the ScanLogic driver and Mac driver will not properly handle it.
   
   if(result != USBS_PASSED)
      {
      failedIn();    // stalls EP8 
      return(USBS_FAILED);
      }

   // no need to do the data xfer phase if the host isn't expecting any data.
   if (!dataTransferLenLSW && !dataTransferLenMSW)
      {
      // Make sure the status is correct
      while (readATAPI_STATUS_REG() & ATAPI_STATUS_BUSY_BIT)
         ;

      if (readATAPI_STATUS_REG() & ATAPI_STATUS_DRQ_BIT)
         return(USBS_PHASE_ERROR);       // USBS_PHASE_ERROR -- Hn < Di (case 2)
      else
         return(USBS_PASSED);
      }

   //////////////////////////////////////////////////////////////////
   // Start of data xfer phase
   //////////////////////////////////////////////////////////////////
   if (useUdma) 
   {
      result = inDataFromDriveUdma();
   }
   else
   {
      result = inDataFromDrive();
   }

   if (dataTransferLen)    
      {
      // Case H(i) > D(i) or H(i) > D(n)
      // "terminate the transfer with a short packet, then STALL the IN endpoint"
      failedIn();       // only stalls EP8, does not return error
     
      // Pass the result to the next layer up.
      return(result);  
      }

   else
      return(result);       // No residue, just return status

#else
   return(0);
#endif
}   


//-----------------------------------------------------------------------------
// Function:  generalSCSIOutCommand()
//
// Input:   none
// Output:  bit flag
//          0 = success, 1 = failure
//
// Global data:
//    CBW from EP2FIFOBUF.
//
// Local data:
//    cmd      - command op code from CBW
//    result   - return status
//
// Description:
//    The scsi command packet is a 12-byte packet extracted from the CBW 
//    contained in EP2FIFOBUF starting from byte 15.  If the command fails, 
//    the OUT endpoint buffer is stalled and the transaction is failed.
//
//    If the command is successful, data is sent to the drive.
//    When the write encounters an error, the endpoint is stalled and the
//    transaction is failed.
//
//-----------------------------------------------------------------------------
BYTE generalSCSIOutCommand()
{
#if DEVICE_TYPE_IS_SCSI
   // Init local vars.

   BYTE result = USBS_FAILED;

   useUdma = 0;

   // if the drive is configured for udma then use udma depending on the
   // scsi command
   if (udmaMode)
   {
      switch(EP2FIFOBUF[0xf])
      {
         case 0x2A:
         case 0xAA:
            useUdma = 1;   //syk
            break;
         default:
            useUdma = 0;
            break;
      }
   }

   result = sendSCSICommand(EP2FIFOBUF + CBW_DATA_START);

   // relinquish control of the bulk buffer occupied by the CBW
   EP2BCL = 0x80;     

   // If the command failed, stall the endpoint and get out
   if (result != USBS_PASSED)
   {
      // If the transfer still contains data, and the xfer has not been terminated by a short packet
      // then we must stop the transfer with a STALL.
      if (dataTransferLen > 0)
      {
         // We may want to stall the endpoint here, but we must be careful to make sure that 
         // we don't set the stall bit when the xfer has completed!
          stallEP2OUT();
      }
      return(USBS_FAILED);
   }

   if (!dataTransferLen)
      return(result);

   if (useUdma)
   {
      result = scsiWriteUdma();
   }
   else
   {
      result = scsiWrite();
   }

   return(result);
#endif

#if DEVICE_TYPE_IS_IDE
   return(0);
#endif
}



#define SENSE_LEN 18
const char code testUnitReady[12] = { 0x00, 0x00, 0x00, 0x00, 00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const char code requestSense[12] = {  0x03, 0x00, 0x00, 0x00, SENSE_LEN, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// Send a TestUnitReady command to the device.
// If the device fails the testUnitReady command, return the sense code, else return 0
BYTE SCSITestUnitReady()
{

   bit result = 0;
   //char count;
    
   if (!scsi)
      return(result);

#if DEVICE_TYPE_IS_SCSI
   useUdma = 0;
   result = sendSCSICommand((char xdata *) testUnitReady);
   if (result != USBS_PASSED)
   {
      result = sendSCSICommand((char xdata *) requestSense);
      if (result != USBS_PASSED)
         {
         }

      //result = waitForIntrq();

      readPIO16toXdata(ATAPI_DATA_REG, halfKBuffer, SENSE_LEN, LISTEN_TO_DRIVE_LEN);
      return(halfKBuffer[12]);
    }
#endif
}   


// Read the Inquiry info into our internal data structures.
// NOT prompted by the host.
#define INQUIRY_LEN 0x2c
const char code inquiryCommand[12] = { 0x12, 0x00, 0x00, 0x00, INQUIRY_LEN, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
void SCSIInquiryToATAPI()
{
#if DEVICE_TYPE_IS_SCSI

    bit result;

    useUdma = 0;
    result = sendSCSICommand((char xdata *) inquiryCommand);
    if (result != USBS_PASSED)
    {
        failedIn();    //This is an internal command, just leave if it fails.
        return;
    }

     result = waitForIntrq();

     readPIO16toXdata(ATAPI_DATA_REG, halfKBuffer, INQUIRY_LEN, LISTEN_TO_DRIVE_LEN);
  

   if (halfKBuffer[SCSI_INQUIRY_DEVICE_CLASS] == 5)
   {
      intrfcSubClass = USB_MS_CD_ROM_SUBCLASS;  
   }
#endif
}   


WORD getDriveDataLen()
{
    WORD driveDataLen;


    driveDataLen = readPIO8(ATAPI_BYTE_COUNT_MSB) << 8;
    driveDataLen += readPIO8(ATAPI_BYTE_COUNT_LSB);
    return(driveDataLen);
}
 


///////////////////////////////////////////////////////////////////////////////
#if DEVICE_TYPE_IS_SCSI
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Function:  scsiWrite()
//
// Input:   none
// Output:  bit flag
//          0 = success, 1 = failure
//
// Global data:
//    dataTransferLen   - Amount of data requested by the host.  Counts down.
//    EP2CS             - Endpoint buffer status byte.
//    EP2BCL            - Endpoint LSB byte count register.
//
// Local data:
//    wDriveDataLen     - Amount of data drive will accept.
//    wAmountToWrite    - Amount of data to write to the drive.  This is typically
//                         a portion wDriveDataLen, and is qualified by the packet lenght.
//    wAmountSent       - Amount of data sent to the drive.  Counts up to ensure we don't
//                         exceed the packet size.
//    driveStatus       - Drive status from last action.  
//    bDone             - loop control flag     
//
// Description:
//    This function handles the special case of scsi MODE PAGE write (MODE SELECT command).
//    The drive byte count and the DRQ bit are used to determine how much data to send.
//    Typically, the host requests to send data that is much larger than what the
//    drive wants to accept.  
//
//    The drive may want to receive data in chunks less than a packet size, and the 
//    total number of bytes to satisfy the command may be less or greater than
//    a packet length.  Drive data request is processed as follows:
//    
//    (1) Total number of bytes is less than a packet, and drive wants to receive it in
//    increments.  The rest of the packet must be processed out of the buffer in
//    the size requested by the drive.  This transaction is governed by the
//    DRQ bit in the status register.
//
//    (2) Total number of bytes is greater than a packet, but drive wants it
//    in increments less than a packet lenght.  Full packet(s) are processed out
//    of the buffer using logic from (1) until the drive is satisfied.  This 
//    transaction is governed by the DRQ bit in the status register as well as the
//    byte count from the drive.  Any data residue in the endpoint buffer is sent 
//    to ATAPI_NULL_REGISTER.
//
//    If the host is determined to queue up packets to the buffer after the drive 
//    byte count has been satisfied (DRQ=0), the data is processed out of the endpoint 
//    buffer and discarded by setting the skip bit in the endpoint's byte count
//    register.
//
//    The DRQ bit is valid.
//
//    If a write encounters an error, we return an error and let caller 
//    stall the endpoint.
//
//-----------------------------------------------------------------------------
static BYTE scsiWrite(void)
{

   WORD wDriveDataLen = 0;
   BYTE driveStatus = 0;
   WORD wAmountToWrite = 0;
   WORD wAmountSent = 0;  
   bit bDone = 0;
   bit bShortPacketReceived = 0;
   BYTE cReturnStatus = USBS_PASSED;
   

   // See if drive is finished processing command and
   // is ready for data.
   while(readATAPI_STATUS_REG() & ATAPI_STATUS_BUSY_BIT)
      ;

   driveStatus = readATAPI_STATUS_REG();

   if (driveStatus & ATAPI_STATUS_DRQ_BIT)
   {
      while( (!(driveStatus & ATAPI_STATUS_ERROR_BIT)) && (!bDone) && dataTransferLen)
         {
          // Make sure the data is already in the endpoint buffer
         if(!(EP2CS & (bmEPEMPTY | bmEPSTALL))) 
            {
            if (EP2BC & (wPacketSize - 1))
               bShortPacketReceived = 1;

            EP2BCL = 00;         // Release the endpoint buffer to FIFO.
   
            // Get the amount of data the drive is willing to accept
            if(!wDriveDataLen)
               wDriveDataLen = getDriveDataLen();
   
            wAmountSent = 0; 
            while( (wPacketSize > wAmountSent) && (!bDone) && dataTransferLen)
               {
               // wAmountToWrite is limited by two factors:
               // -- wDriveDataLen -- The amount of data remaining in the drive's most recent request
               // -- wPacketSize-wAmountSent -- The amount of data remaining in our buffer
               wAmountToWrite = min(wPacketSize-wAmountSent, wDriveDataLen);
               wAmountToWrite = min(dataTransferLen, wAmountToWrite);
      
               // Send the packet and adjust counts.  
               writePIO16(ATAPI_DATA_REG, wAmountToWrite+1);
               dataTransferLen -= wAmountToWrite;
               wDriveDataLen -= wAmountToWrite;
               wAmountSent += wAmountToWrite;
   
               // Make sure the write is successful, otherwise get out. -- First wait for the busy bit to clear.
               while(readATAPI_STATUS_REG() & ATAPI_STATUS_BUSY_BIT)
                  ;
   
               // Don't trust the first result after the busy bit goes away.  Read it again.
               // The DELL-07 CDRW has occasional problems if this is not done.
               driveStatus = readATAPI_STATUS_REG();
               
               if(driveStatus & ATAPI_STATUS_ERROR_BIT)
                  {
                  cReturnStatus = USBS_FAILED;  
                  bDone = 1;                    
                  }
               else
                  {
                  // Check if drive is finished with transaction.
                  if(!(driveStatus & ATAPI_STATUS_DRQ_BIT))
                     {
                     bDone=1;
                     cReturnStatus = USBS_PASSED;
                     }
                  else if(!wDriveDataLen)
                     wDriveDataLen = getDriveDataLen();
   
                  } // end else
               } // end while(within current packet)
            } // end if(data in endpoint buffer)
         } // end while(all data)
      } // end if (drive has data)

   // If the device still wants more data from the host at this point, it's a phase error (case 13)
   if ((readATAPI_STATUS_REG() & ATAPI_STATUS_DRQ_BIT) || wDriveDataLen)
      cReturnStatus = (USBS_PHASE_ERROR);   

   // If there is still data in our buffer there are several possibilities:
   // 1)  Data in our buffer.  No more data coming from the host.  Reset the endpoint.
   // 2)  Buffer full, more data expected from the host.  STALL.
   // 3)  Data still on the way from the host that will go beyond our buffer size.  STALL.
   // 4)  Data still on the way from the host that will fit within our buffer size.  Wait for the data, then reset the endpoint.
   //       There is no clean way to wait for the data that works with the current TCL scripts.  Just reset the endpoing.
   //
   if (dataTransferLen && !bShortPacketReceived)
      stallEP2OUT();
   // This is done in send_usbs()
   //   else if (!(EP2468STAT & bmEP2EMPTY))         // Discard residue in the buffer if needed.  This is required so that the FIFO will give the buffer back to us (if we DON'T stall)
   //                           // For example, case 11 with 250 bytes of data.
   //      ResetAndArmEp2();     
   return(cReturnStatus);
}


//-----------------------------------------------------------------------------
// Function:  sendSCSICommand()
//
// Input:   
//    bit useInterrupt     
//    char xdata *cmdbuf   - scsi command packet (EP2FIFOBUF + CBW_DATA_START)
//
// Output:  bit flag
//          0 = success, 1 = failure
//
// Local data:
//    WORD cmd_data - modified command data packet
//
// Description:
//    The command packet is sent to the drive using 16-bit register writes to
//    the drive's data register.  Data is modified to handle endian-ness before
//    before it is sent to the drive.
//
//    ALERT!!!:  Sending the command packet to the drive using register writes was
//       due to an assumption that we could not GPIF the middle of an endpoint's
//       content.  From SSW, we can walk down an endpoint's content using GPIF
//       as long as the transaction count is not exhausted.  For optimization,
//       we may want to GPIF the start of the CBW (maybe send it to the null
//       register where it won't cause grief), GPIF the scsi command packet
//       to the drive then discard the rest of the CBW.  This would remove the
//       need to process the command packet for endian-ness.
//-----------------------------------------------------------------------------
static bit sendSCSICommand(char xdata *cmdbuf)
{

   BYTE driveStatus;
  
   prepareForATAPICommand();
   
   CLEAR_INTRQ;    // Clear the interrupt bit.
   
   // Make sure the device is ready for the packet
   while (readATAPI_STATUS_REG() & ATAPI_STATUS_BUSY_BIT)
      ;
   
   // Send the "ATAPI packet" command
   writePIO8(ATAPI_COMMAND_REG, ATAPI_COMMAND_ATAPI_PACKET);
   
   // Wait for the register block to be non-busy and to request data
   do
     driveStatus = readPIO8(ATAPI_STATUS_REG);
   while( driveStatus & ATAPI_STATUS_BUSY_BIT);
   
   if(driveStatus & ATAPI_STATUS_ERROR_BIT)
     return(USBS_FAILED);
   
   // Write 6 words of command
   {
   BYTE i;
   WORD cmd_data;

      for(i=0; i<12; i+=2)
         {
         cmd_data = (BYTE) (cmdbuf[i+1]);
         cmd_data = (cmd_data << 8) | (BYTE) (cmdbuf[i]);
         writePIO8(ATAPI_DATA_REG, cmd_data);
         }
   }
   
   if (useUdma)
      return(USBS_PASSED);

   {
     // Wait for the register block to be non-busy   
     for (driveStatus=ATAPI_STATUS_BUSY_BIT; driveStatus & ATAPI_STATUS_BUSY_BIT; )
         {
         // Read the alt status register so we don't trigger any events
         driveStatus = readPIO8(ATAPI_ALT_STATUS_REG);
         }
   }

   if (readATAPI_STATUS_REG() & ATAPI_STATUS_ERROR_BIT)  // Reading the status reg clears the interrupt
      return(USBS_FAILED);
   else
      return(USBS_PASSED);
}   



static void prepareForATAPICommand()
{


   // Select ATAPI device
   writePIO8(ATAPI_DRIVESEL_REG, 0xa0);

   // Make sure the device is ready for the packet
   while(readATAPI_STATUS_REG() & ATAPI_STATUS_BUSY_BIT)
      ;

   // configure the transfer mode (PIO or UDMA) using the feature register
   if (useUdma)
   {
      writePIO8(ATAPI_FEATURE_REG, 0x01);
   }
   else
   {
      // This disables disconnect/reconnect, synchronous, overlapped and DMA features.
      writePIO8(ATAPI_FEATURE_REG, 0x00);
   }

   // Set "max byte count"
   writePIO8(ATAPI_BYTE_COUNT_LSB, 0xff);
   writePIO8(ATAPI_BYTE_COUNT_MSB, 0xff);

}



//-----------------------------------------------------------------------------
// Function:  inDataFromDrive()
//
// Input:   none
// Output:  bit flag
//          0 = success, 1 = failure
//
// Global data:
//
// Local data:
//
// Description:
// Read from the drive until the drive is empty or the buffer is full.
// This breaks up the 16 bit length read into wPacketSize. driveDataLen is
// the amount of data available from the drive.  If the last read results in 
// a short packet, but the drive has more data to give, don't release the packet.
// until either the drive has finished or the packet is full.
//
// Phase error note:  In the Hi<>Do case, we will send garbage to the host before
// reporting the phase error.  In the Hi<Di case we return Di and report phase error.
//-----------------------------------------------------------------------------
static BYTE inDataFromDrive()
{

   BYTE driveStatus;
   WORD wDriveDataLen = 0;
   WORD wReadLen = 0;       // Amount of data in the current transaction w/ the drive
   WORD wInSize = 0;        // Tracks the amount of data in the current IN packet
   bit bDone = 0;
   bit cReturnStatus = USBS_FAILED;

   // Clear the interrupt bit
   CLEAR_INTRQ;
 
   // See if drive is finished processing command and
   // is ready for data.
   do
       {
       driveStatus = readPIO8(ATAPI_STATUS_REG);
       }
   while( driveStatus & ATAPI_STATUS_BUSY_BIT );   // DO-WHILE!!!

   // If the drive doesn't have data for us, take off!
   if(!(driveStatus & ATAPI_STATUS_DRQ_BIT) )
      return (USBS_PASSED);

   while( (!(driveStatus & ATAPI_STATUS_ERROR_BIT)) && (!bDone) && dataTransferLen)
      {
      // Get the amount of data the drive is willing to accept
      if(!wDriveDataLen)
         wDriveDataLen = getDriveDataLen();

      // Wait for an available buffer.
      waitForInBuffer();

      while( (wInSize < wPacketSize) && (!bDone) && dataTransferLen)
         {
         // Get data from drive to endpoint.
         wReadLen = min(wPacketSize-wInSize, wDriveDataLen);
         wReadLen = min(dataTransferLen, wReadLen);      // added to cover Hi<Di case
         readPIO16(ATAPI_DATA_REG, wReadLen+1);          // add 1 in case readLen is odd
   
         // Adjust counts.
         wDriveDataLen -= wReadLen;
         dataTransferLen -= wReadLen;
         wInSize += wReadLen;

         // Wait for the register block to be non-busy
         while(readATAPI_STATUS_REG() & ATAPI_STATUS_BUSY_BIT)
            ;
         
         // Don't trust the first result after the busy bit goes away.  Read it again.
         // The DELL-07 CDRW has occasional problems if this is not done.
         driveStatus = readATAPI_STATUS_REG();

         // Check if we're done or the drive still has data to transfer.
         if(driveStatus & ATAPI_STATUS_ERROR_BIT)
            {
            bDone = 1;
            cReturnStatus = USBS_FAILED;
            }

         else if(!(driveStatus & ATAPI_STATUS_DRQ_BIT) )
            {
            bDone=1;      // No more data
            cReturnStatus = USBS_PASSED;
            }

         else 
            if(!wDriveDataLen)
               wDriveDataLen = getDriveDataLen();
         }

      // We either have a full packet or we're at the last packet.
      // Release the buffer to the SIE and re-init packet byte count.

      EP8BCH = MSB(wInSize);
      EP8BCL = LSB(wInSize);
      if (wInSize < wPacketSize)
         bShortPacketSent = 1;
      wInSize = 0;
      }

   // If the device still says it has data ready for us, we're either in case
   // 7 (Hi<Di) or 8 (Hi <> Do).  Both are phase error cases.
   if ((readATAPI_STATUS_REG() & ATAPI_STATUS_DRQ_BIT) || wDriveDataLen)
      return (USBS_PHASE_ERROR);
  
   return(cReturnStatus);
}   

static bit inDataFromDriveUdma()
{
   BYTE driveStatus;
   BYTE error;
   
   initUdmaRead();

   readUDMA((dataTransferLen + 1) >> 1);

   // If there's anything in the transfer count it's an error
   // This code doesn't handle partial transfers.  Any failure kills the entire xfer.
   if (! (GPIFTCMSW || GPIFTCLSW))
   {
      dataTransferLen = 0;
   }

   // switch the EP back to manual mode
   EP8FIFOCFG = 0x05;

   driveStatus = readATAPI_STATUS_REG();

   // Check if stall is needed
   if (dataTransferLenLSW & (wPacketSize - 1))
      bShortPacketSent = 1;

   if (driveStatus & ATAPI_STATUS_ERROR_BIT) 
   {
      error = readPIO8(ATAPI_ERROR_REG);

      // Upper 4 bits of error contain sense key.  4 is the sense key for parity error
      if ((error & 0xf0) == 0x40)
      {
        // Allow up to a 10/1 error ratio.  If it gets above that, slow down or stop using UDMA.
         udmaErrorCount += 10;
         if (udmaErrorCount >= 20)
            {
            if (udmaMode == TRANSFER_MODE_UDMA4)
               {
               udmaMode = TRANSFER_MODE_UDMA2;
               configureATATransferMode(udmaMode);
               }
            else
               {
               udmaMode = 0;
               configureATATransferMode(PIO_MODE4);
               }
            udmaErrorCount = 0;
            }
      }
      return(USBS_FAILED);
   }
   else if (driveStatus & ATAPI_STATUS_DRQ_BIT)
      return(USBS_PHASE_ERROR);
   else
      {
      if (udmaErrorCount)
         udmaErrorCount--;

      return(USBS_PASSED);
      }
}
   

// Note:  This routine will never STALL the out pipe.
//        If dataTransferLen remains above 0, it will get stuck rather than stall.
static BYTE scsiWriteUdma()
{
   BYTE driveStatus;

   writeUDMA((dataTransferLen+1) >> 1);

   // If there's anything in the transfer count it's an error
   // This code doesn't handle partial transfers.  Any failure kills the entire xfer.
   if (! (GPIFTCMSW || GPIFTCLSW))
   {
      dataTransferLen = 0;
   }

   // Check status to clear interrupt.
   driveStatus = readATAPI_STATUS_REG();
   if (driveStatus & ATAPI_STATUS_ERROR_BIT) 
      return(USBS_FAILED);
   else if (driveStatus & ATAPI_STATUS_DRQ_BIT)
      return(USBS_PHASE_ERROR);
   else
      return(USBS_PASSED);
}

////////////////////////////////////////////////////////////////////////////////////////
#endif      // DEVICE_TYPE_IS_SCSI
////////////////////////////////////////////////////////////////////////////////////////







