#pragma NOIV               // Do not generate interrupt vectors
//-----------------------------------------------------------------------------
//   File:      periph.c
//   Contents:   Hooks required to implement USB peripheral function.
//
//   Copyright (c) 1997 AnchorChips, Inc. All rights reserved
//
// $Archive: /USB/atapifx2/software/periph.c $
// $Date: 1/23/02 9:36a $
// $Revision: 58 $
//-----------------------------------------------------------------------------
#include "fx2.h"
#include "fx2regs.h"
#include "gpif.h"
#include "atapi.h"

extern BOOL   Sleep;
extern BOOL   Rwuen;
extern BOOL   Selfpwr;

void clearResetLine();
void clearResetLine();

WORD cbwTagLow;          // Tag from the most recent CBW packet
WORD cbwTagHi;
BYTE currentState;

DWORD dataTransferLen;
DWORD driveCapacity;
bit scsi;

BYTE xdata SCSIInquiryData[44];
BYTE code SCSIInquiryDataSource[44] = 
{
0x00,           //= Device class
//0x0e,           // Device class RBC
0x00,           //= RMB bit is set by inquiry data
0x00,           //
0x01,           //= Data format = 1
0x00,           //= Additional length   (changed to 0 from 0x75)
0x00, 0x00, 0x00,   //
0x43, 0x79, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, // = Manufacturer "Cypress "
0x41, 0x54, 0x41, 0x50, 0x49, 0x20, 0x52, 0x65, 0x66, 0x20, 0x44, 0x65, 0x73, 0x69, 0x67, 0x6e, // = Product(MS Ref Design)
0x30, 0x31, 0x2E, 0x30, // = Revision
0x30, 0x39, 0x2F, 0x32, 0x34, 0x2F, 0x39, 0x38, // = Vendor unique (chopped off)

};

//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//   The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

void TD_Init(void)             // Called once at startup
{
   // set the CPU clock to 48MHz
   CPUCS = ((CPUCS & ~bmCLKSPD) | bmCLKSPD1) ;
   WRITEDELAY();

   // set the slave FIFO interface to 48MHz
   IFCONFIG |= 0x40;
   WRITEDELAY();

   // init state/reset variables
   currentState = UNCONFIGURED;

   initUSB();                 // configure output ports and endpoint params
   mymemmovexx(&GPIF_WAVE_DATA, (BYTE xdata *) WaveDataPio0, 64);          // load wave forms in memory
   mymemmovexx(SCSIInquiryData, (BYTE xdata *) SCSIInquiryDataSource, sizeof(SCSIInquiryDataSource));

   resetATAPIDevice();

   if (SCSITestUnitReady())
      SCSITestUnitReady();


   ATAPIIdDevice();        // Get serial number

   if (SCSITestUnitReady())
      SCSITestUnitReady();

   intrfcSubClass = USB_MS_SCSI_TRANSPARENT_SUBCLASS;  // IDE devices are treated as transparent SCSI.  Normally would use RBC, but it's not in Microsoft's INF file.
   if (scsi) 
   {
      SCSIInquiryToATAPI();            // Set intrfcSubClass to tell Zip and MO from CD-ROM
   }
}


char const code usbcString[] = "USBC";
void TD_Poll(void)             // Called repeatedly while the device is idle
{
   WORD    count = 0;

   // check EP2 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
   if(!(EP2CS & bmEPEMPTY))   
      { 
      // Check for "USBC"
      if ( *((DWORD xdata *)EP2FIFOBUF) != *((DWORD xdata *) usbcString))
         {
         // error -- Stall the endpoint no matter what
         EP2CS = bmEPSTALL;
         }
      else
         {
         if (EP2BC < EP2FIFOBUF[CBW_CBW_LEN]+CBW_DATA_START)
            {
             // Error -- Stall the endpoint
            EP2CS = bmEPSTALL;
            }
         else            
            {
            // Good packet, forward to the device.
            processCBW();  
            }
         }
      }
}


//-----------------------------------------------------------------------------
// Support functions for specific device
//-----------------------------------------------------------------------------

void initUSB(void)
{

   PORTACFG = 0x01;       // set up PORTA for port I/O  
   OEA = 0xBE;            // PORTA is all output except PA.0(INTRQ) AND PA.6(DASP#)
   OUTATAPI = ATAPI_IDLE_VALUE;  // includes RESET line PA.0

   // Make Interrupt 0 level triggered
   IT0 = 0;

   //CPUCS = 0x0c;        // set clock to 24MHz.      
   IFCONFIG = 0x8A | bmGSTATE;      // b7=0: use FCLK pin to clock FIFO, GPIF; =1: use internal 30/48 MHz clock(=30)
   CPUCS = 0x14;        // set clock to 48MHz.      
   //IFCONFIG = 0xcA;      // b7=0: use FCLK pin to clock FIFO, GPIF; =1: use internal 30/48 MHz clock(=48)
   FIFOPINPOLAR = 0x00;    // ff pin is active low

   PINFLAGSAB = 0x00;    // FLAGA PF for FIFO selected by FIFOADR[1..0]
   PINFLAGSCD = 0x00;    // FLAGB FF for FIFO selected by FIFOADR[1..0]

   // Set up interrupt parameter
//   INTSETUP |= bmINT4;  // Allow FIFO and GPIF interrupts

   // GPIF and CTL configuration
   GPIFCTLCFG = 0x00;   // 
   GPIFIDLECTL = 0x77;  // x111x111 - CTL3 not enabled
                        // ||||||||_CTL0 = 1 during idle
                        // |||||||__CTL1 = 1 during idle
                        // ||||||___CTL2 = 1 during idle
                        // ||||_____CTL0 output enable
                        // |||______CTL1 output enable
                        // ||_______CTL2 output enable
                        // 
   GPIFIDLECS = 0;      // tristate data bus during idle interval
   GPIFWFSELECT = (2 << 6) | (3 << 4) | (0 << 2) | (1);    // Single write is 2, Single read is 3, write is 0, Read is waveform 1 

   // Endpoint initialization
   EP2CFG = 0xA0;           // ep2 is valid BULK OUT 512 quad buffered
   EP2FIFOCFG = 0x05;       // WORDWIDE=1M MANUAL
   EP2FIFOPFH = 0x00;       // PF=0 when BC > PF -> Decis=0 (1 byte in FIFO)
   EP2FIFOPFL = 0x00;       // PF and BC refer to the current pkt -> PKTSTAT=0
   EP2GPIFPFSTOP = 0;       // Do not stop on PF

   EP8CFG = 0xE0;          // ep8 is valid BULK IN 512 double buffered
   EP8FIFOCFG = 0x05;      // set EP8:  0x05=MANUAL, 0x0D=AUTOIN

   // mark all unused endpoints invalid - setting each reg to 0x22 instead of just clearing
   // the valid bit to save code space.  0x22 basically sets all of these endpoints to 
   // not valid, bulk, double 512 buffered.
   EP1OUTCFG = EP1INCFG = EP4CFG = EP6CFG = 0x22;

   // disbable Auto Arm
   REVCTL |= bmNOAUTOARM;

   // arm the OUT endpoint.  By default OUT endpoints come up unarmed.
   ResetAndArmEp2();

}



// Stalls EP2OUT endpoint.
void stallEP2OUT()
{
   // Check to see if stall is needed.  If it is, STALL the endpoint.
   // After we have set the STALL, make sure we didn't get the last packet while we were STALLing.
   WORD x;

   if (EP2468STAT & bmEP2EMPTY)
      x = 0;
   else
      x = EP2FIFOBCL + (EP2FIFOBCH << 8) + EP2BC;

//   if (dataTransferLen > ((x + 1) & 0xfffe))     // Round up to allow for odd xfer lengths
   if (dataTransferLen > x)     

   {
      EP2CS |= bmEPSTALL;

      EZUSB_Delay(100);

      if (EP2CS & bmEPSTALL)
         x=1234;

      // If the host has already cleared the STALL, the EP will be empty here, but we will drop safely through the if()
      if (EP2468STAT & bmEP2EMPTY)
         x = 0;
      else
         x = EP2FIFOBCL + (EP2FIFOBCH << 8) + EP2BC;
   
      if (dataTransferLen > x)     
         {
         ResetAndArmEp2();    // Stall no longer needed
         EP2CS = 0;           // Clear stall bit
         }
   }
}   

void processCBW()
{
   // Save the tag for use in the response
   cbwTagLow = *((WORD volatile xdata*)(EP2FIFOBUF+CBW_TAG));
   cbwTagHi =  *((WORD volatile xdata*)(EP2FIFOBUF+CBW_TAG+2));

   // Get the length (convert from little endian)
   *(((BYTE *) &dataTransferLen)+3) = (EP2FIFOBUF+CBW_DATA_TRANSFER_LEN_LSB)[0];  // "Residue"
   *(((BYTE *) &dataTransferLen)+2) = (EP2FIFOBUF+CBW_DATA_TRANSFER_LEN_LSB)[1];  // "Residue"
   *(((BYTE *) &dataTransferLen)+1) = (EP2FIFOBUF+CBW_DATA_TRANSFER_LEN_LSB)[2];  // "Residue"
   *(((BYTE *) &dataTransferLen)+0) = (EP2FIFOBUF+CBW_DATA_TRANSFER_LEN_LSB)[3];  // "Residue"

//   writePIO8(ATAPI_NULL_REG, dataTransferLenLSW);

   // Our personal "firmware update" command
   if (EP2FIFOBUF[0xf] == 0xfb && !(EP2FIFOBUF[CBW_FLAGS] & CBW_FLAGS_DIR_BIT))
      {
      // relinquish control of the bulk buffer occupied by the CBW
      EP2BCL = 0x80;     

      // Write the EEPROM
      EEPROMWrite(dataTransferLenLSW);
      sendUSBS(USBS_PASSED);
      }
   else if (EP2FIFOBUF[0xf] == 0xfa && (EP2FIFOBUF[CBW_FLAGS] & CBW_FLAGS_DIR_BIT))
      {
      extern BYTE code StringDscr3;
      BYTE len = (&StringDscr3)[0]<<1;

      len = max(len, dataTransferLenLSW);

      // relinquish control of the bulk buffer occupied by the CBW
      EP2BCL = 0x80;     

      mymemmovexx(EP8FIFOBUF, (char xdata *)(&StringDscr3)+2, len);
      waitForInBuffer();
      EP8BCH = 0; 
      EP8BCL = len; 
      sendUSBS(USBS_PASSED);
      }
   else if (EP2FIFOBUF[CBW_FLAGS] & CBW_FLAGS_DIR_BIT || !dataTransferLen)
      {
      currentState = RECEIVED_IN_CMD;
      if (scsi)
         sendUSBS(generalSCSIInCommand());
      else
         sendUSBS(generalIDEInCommand());
      }
   else
       {
       currentState = RECEIVED_OUT_CMD;
       if (scsi)
           sendUSBS(generalSCSIOutCommand());
       else
           sendUSBS(generalIDEOutCommand());
       }
   currentState = WAIT_FOR_CBW;
}   



void sendUSBS(BYTE passOrFail)
{
   bit done = 0;
   
   // generalIDEx/generalSCSIx command returns here with passOrFail status bit
   // which is re-cast as the error byte of CSW

   while (!done)
   {
      if( (!(EP8CS & bmEPFULL)) && (!(EP8CS  & bmEPSTALL)))      // Wait for an available buffer
      {

         // check to see if there is any data in our OUT endpoint.  If there is,
         // do a fifo reset before sending the CSW.  How could this happen?  If we
         // stalled this CBW, it is possible that the host sent us data after the
         // the CBW.
         if (!(EP2CS & bmEPEMPTY))
         {
            ResetAndArmEp2();
         }

         // Fill the buffer & send the data back to the host
         EP8FIFOBUF[0] = 'U';        // Believe it or not, this is pretty efficient!
         EP8FIFOBUF[1] = 'S';
         EP8FIFOBUF[2] = 'B';
         EP8FIFOBUF[3] = 'S';

         *((WORD volatile xdata*)(EP8FIFOBUF+CBW_TAG)) = cbwTagLow;
         *((WORD volatile xdata*)(EP8FIFOBUF+CBW_TAG+2)) = cbwTagHi;
         
         // have to store LSB first
         EP8FIFOBUF[8+0] = ((BYTE *)&dataTransferLen)[3];    // "Residue"
         EP8FIFOBUF[8+1] = ((BYTE *)&dataTransferLen)[2];    // "Residue"
         EP8FIFOBUF[8+2] = ((BYTE *)&dataTransferLen)[1];    // "Residue"
         EP8FIFOBUF[8+3] = ((BYTE *)&dataTransferLen)[0];    // "Residue"
         
         *((BYTE xdata *) (EP8FIFOBUF+12)) = passOrFail;                 // Status
         EP8BCH = 0;
         EP8BCL = 13;
         done = 1;
      }
   }
}   

void failedIn()
{
   // Stall if the host is still expecting data.  Make sure
   // endpoint is empty before doing the stall.

   if (dataTransferLen /*&& !bShortPacketSent*/)
      {
      while( !(EP8CS & bmEPEMPTY) )
         ;

      //EP8BCH = 0;    // Terminate with NULL packet, not STALL.
      //EP8BCL = 0;
      EP8CS = bmEPSTALL; // TPM
      }
}

// Read data from the drive
// Issues repeated calls to readPIO16 to pull in multiple blocks
// of data from the drive
// Returns amount of data reported by drive else returns 0
WORD readPIO16toXdata(char addr, char xdata *inbuffer, WORD count, bit ignoreDriveLen)
{
    WORD driveDataLen = 0;
    WORD saveDriveDataLen;
    BYTE driveStatus;
    WORD timeout = PROCESS_CBW_TIMEOUT_RELOAD;
   WORD i;

    for (driveStatus = 0; !(driveStatus & ATAPI_STATUS_DRQ_BIT) && (timeout-- > 0); )
        driveStatus = readATAPI_STATUS_REG();
    if (!timeout)
        return(count);

    if (timeout == 0xffff)
        return(0);

    if (ignoreDriveLen)
      {
      count = saveDriveDataLen = ATA_SECTOR_SIZE;
      driveDataLen = 0;
      }

   if (scsi)
   {      
      saveDriveDataLen = driveDataLen = getDriveDataLen();
      count = min(count, driveDataLen);
   }
   else
   {
      saveDriveDataLen = driveDataLen = ATA_SECTOR_SIZE;
      count = min(count, driveDataLen);
      driveDataLen -= count;
   }


   for (i=0; i<count; i+=2)
   {
      ((WORD*)inbuffer)[0] = readWordPIO8(addr);
      inbuffer +=2;
   }

//    while (count)
//        {
//        readLen = min(count, 0x200);         // Read and write routines are limited to one USB buffer size
//        readPIO16(addr, EP8FIFOBUF, readLen);
//        mymemmovexx(inbuffer, EP8FIFOBUF, readLen);
//       count -= readLen;
//        inbuffer += readLen;
//        }
        
   // For IDE, must empty the buffer after the relavent data has been read.
   if (!scsi)
      while (driveDataLen)
      {
         readPIO16(addr, 2);
         driveDataLen -= 2;
      }

    return(saveDriveDataLen);
}   

bit waitForBusyBit()
{
    BYTE driveStatus;

    do
    {
        driveStatus = readATAPI_STATUS_REG();
    }
    while((driveStatus & (ATAPI_STATUS_BUSY_BIT)));    // Do-while
    
    // Some drives clear the busy bit asynchronously.  Read the reg one more time to be sure.
    driveStatus = readATAPI_STATUS_REG();

    if ((driveStatus & ATAPI_STATUS_ERROR_BIT))
        return(USBS_FAILED);
    else
        return(USBS_PASSED);
}   

void mymemmovexx(BYTE xdata * dest, BYTE xdata * src, WORD len)
{
    while (len--)
    {
        *dest++ = *src++;
    }
}    


// Wait for interrupt from the drive.
// Clears the interrupt source on the drive, the INT0 is level triggered, so it clears automagically .
// 
// Returns:
//      USBS_FAILED -- Timeout
bit waitForIntrq()
{
    int timeout;

    // Make sure the drive has data ready for us
    for (timeout = PROCESS_CBW_TIMEOUT_RELOAD;
        timeout && IE0; 
        timeout--)
        ;

    // Clear the interrupt source
    readATAPI_STATUS_REG();

    return(USBS_PASSED);
}


//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//   The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

// Setup Data Available Interrupt Handler
void ISR_Sudav(void) interrupt 0
{
   SetupCommand();
   EZUSB_IRQ_CLEAR();
   INT2CLR = bmSUDAV;         // Clear SUDAV IRQ
}


void ISR_Ures(void) interrupt 0
{
   // whenever we get a USB reset, we should revert to full speed mode
   pConfigDscr = pFullSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
   pOtherConfigDscr = pHighSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
   wPacketSize = FS_BULK_PACKET_SIZE;
   EP8FIFOPFH = 0x80;
   EP8FIFOPFL = 0x60;

   EP8AUTOINLENH = MSB(wPacketSize);
   EP8AUTOINLENL = LSB(wPacketSize);


   FIFORESET = 8;
   
   ResetAndArmEp2();

   // clear the stall and busy bits that may be set
   EP2CS = 0;     // set EP2OUT to empty and clear stall
   EP8CS = 0;     // set EP8OUT to empty and clear stall


   EZUSB_IRQ_CLEAR();   
   INT2CLR = bmURES;        // Clear URES IRQ
   
   if (currentState != UNCONFIGURED)
   {
      EA = 0;
      // force a soft reset after the iret.
      softReset();
   }

}

void ISR_Susp(void) interrupt 0
{
   Sleep = TRUE;
   EZUSB_IRQ_CLEAR();
   INT2CLR = bmSUSP;
}

void ISR_Highspeed(void) interrupt 0
{
   if (EZUSB_HIGHSPEED())
   {
      pConfigDscr = pHighSpeedConfigDscr;
      ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
      pOtherConfigDscr = pFullSpeedConfigDscr;
      ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
      wPacketSize = HS_BULK_PACKET_SIZE;
      EP8FIFOPFH = 0x89;
      EP8FIFOPFL = 0x90;
   }

   EP8AUTOINLENH = MSB(wPacketSize);
   EP8AUTOINLENL = LSB(wPacketSize);


   EZUSB_IRQ_CLEAR();
   INT2CLR = bmHSGRANT;
}

#define FW_STRETCH_VALUE_5 5

void ResetAndArmEp2()
{
   // adjust stretch to allow for synchronization delay.  We are about
   // to do several back to back writes to registers that require a
   // synchroniztion delay.  Increasing stretch allows us to meet
   // the delay requirement.  See "Synchroniztion Delay" in the Technical
   // Reference Manual for more information
   // Set the stretch to 5
   CKCON = (CKCON&(~bmSTRETCH)) | 5;

   FIFORESET = 2;

   // we're quad-buffered, so we need to arm EP2 four times
   EP2BCL = 0x80;
   EP2BCL = 0x80;
   EP2BCL = 0x80;
   EP2BCL = 0x80;

   // Reset the stretch to 0
   CKCON = (CKCON&(~bmSTRETCH)) | FW_STRETCH_VALUE;
}
