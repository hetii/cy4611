//-----------------------------------------------------------------------------
//  Copyright (c) 1999 Cypress Semiconductor, Inc. All rights reserved
//-----------------------------------------------------------------------------
//
// This file contains GPIF support code.  
//
// $Archive: /USB/atapifx2/software/gpif.c $
// $Date: 1/23/02 9:37a $
// $Revision: 62 $
//-----------------------------------------------------------------------------
#pragma ot(8,SPEED) // keep gpif.c at Optimization Level 8 (no re-ordering)
					// This is because some DVD-RAM drives had a problem when a common
					// subroutine was replaced. The problem appeared to be only a minor
					// difference in timing. The DVD-RAM drive would fail if a subroutine
					// call was added to readPIO8. It reported a residue of data after
					// a prior transfer had been completed, and when that data was read,
					// it was just a buffer filled with invalid (repeating) data.
					// The problem appeared to be the fault of the DVD-ROM drive.

#include <scsi.h>
#include "fx2.h"
#include "fx2regs.h"
#include "gpif.h"


#ifdef GPIF_ABORT_BUG_PRESENT
const char code AbortWave[32] =                                                           
{ 
/* LenBr */ 0x01,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Opcode*/ 0x00,     0x07,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Output*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* LFun  */ 0x00,     0x40,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00    
};
#endif
//
// Do not edit the following waveform array!  Use the GPIF utility to edit the file
// GPIFPIO0.C.  Once modifications are made there, paste the resulting waveform
// array into this file.
//
const char code WaveDataPio0[128] =                                                           
{ 
// Wave 0                                                                                  
/* LenBr */ 0x0A,     0x01,     0x0A,     0x3F,     0x00,     0x00,     0x00,     0x00,    
/* Opcode*/ 0x02,     0x02,     0x04,     0x01,     0x00,     0x00,     0x00,     0x00,    
/* Output*/ 0xFE,     0xFF,     0xFF,     0xFF,     0x00,     0x00,     0x00,     0x00,    
/* LFun  */ 0x00,     0x00,     0x00,     0x76,     0x00,     0x00,     0x00,     0x00,    
// Wave 1                                                                                  
/* LenBr */ 0x08,     0x0A,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Opcode*/ 0x00,     0x06,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Output*/ 0xFD,     0xFF,     0xFF,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* LFun  */ 0x00,     0x00,     0x36,     0x00,     0x00,     0x00,     0x00,     0x00,    
// Wave 0  UDMA Write
/* LenBr */ 0x08,     0x01,     0x05,     0x1C,     0x03,     0x01,     0x02,     0x01,    
/* Opcode*/ 0x01,     0x00,     0x02,     0x03,     0x02,     0x02,     0x26,     0x00,    
/* Output*/ 0xFF,     0xFB,     0xFA,     0xFA,     0xF8,     0xF9,     0xC3,     0xFF,    
/* LFun  */ 0x09,     0x09,     0xA8,     0x09,     0x00,     0x00,     0x36,     0x3F,     
// Wave 1 UDMA Read
/* LenBr */ 0x08,     0x01,     0x13,     0x05,     0x01,     0x02,     0x01,     0x01,    
/* Opcode*/ 0x01,     0x00,     0x01,     0x00,     0x00,     0x26,     0x26,     0x00,    
/* Output*/ 0xFF,     0xFB,     0xF8,     0xFB,     0xFB,     0xFB,     0xFF,     0xFF,    
/* LFun  */ 0x09,     0x09,     0x09,     0x09,     0x00,     0x2D,     0x36,     0x3F,    
};                                                                                         

//
// Do not edit the following waveform array!  Use the GPIF utility to edit the file
// GPIFPIO4.C.  Once modifications are made there, paste the resulting waveform
// array into this file.
//
// Don't forget that the IORDY signal can be produced at the drive up to 35ns after we
// drop RD or WR.  It then takes 2 clocks for us to demet the signal (async input).  The
// IORDY signal is available on the FIFTH clock edge after RD or WR is dropped.
// Edge 1)  Drop RD/WR (start state 0)
// Edge 2)  No action
// Edge 3)  IORDY detected at first flop
// Edge 4)  IORDY detected at second flop
// Edge 5)  IORDY available as an input to IF/THEN statements. (start state 1)
//
const char code WaveDataPio4[128] =                                                           
{                                                                                          
// Wave 0                                                                                  
/* LenBr */ 0x04,     0x11,     0x01,     0x3F,     0,0,0,0,
/* Opcode*/ 0x02,     0x03,     0x02,     0x07,     0,0,0,0,
/* Output*/ 0xFE,     0xFE,     0xFF,     0xFF,     0,0,0,0,
/* LFun  */ 0x00,     0x00,     0x00,     0x36,     0,0,0,0,
// Wave 1                                                                                  
/* LenBr */ 0x04,     0x11,     0x3f,     0x00,     0,0,0,0,
/* Opcode*/ 0x00,     0x01,     0x07,     0x00,     0,0,0,0,
/* Output*/ 0xFD,     0xFD,     0xFF,     0x00,     0,0,0,0,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0,0,0,0,
// Wave 0  UDMA Write
/* LenBr */ 0x08,     0x01,     0x06,     0x1C,     0x01,     0x01,     0x02,     0x07,    
/* Opcode*/ 0x01,     0x00,     0x02,     0x03,     0x02,     0x02,     0x26,     0x00,    
/* Output*/ 0xFF,     0xFB,     0xFA,     0xFA,     0xF8,     0xFB,     0xFB,     0xFF,    
/* LFun  */ 0x09,     0x09,     0x12,     0x09,     0x00,     0x2d,     0x36,     0x3F,     
// Wave 1  UDMA Read
/* LenBr */ 0x08,     0x01,     0x13,     0x01,     0x01,     0x02,     0x01,     0x07,    
/* Opcode*/ 0x01,     0x00,     0x01,     0x00,     0x00,     0x26,     0x26,     0x00,    
/* Output*/ 0xFF,     0xFB,     0xF8,     0xFB,     0xFB,     0xFB,     0xFF,     0xFF,    
/* LFun  */ 0x09,     0x09,     0x09,     0x1b,     0x00,     0x2D,     0x36,     0x3F,    
};                                                                                         

void hardwareReset()
{
    OUTATAPI &= ~ATAPI_RESET_;
    EZUSB_Delay(100);
    OUTATAPI |= ATAPI_RESET_;
}   


void initUdmaRead()
{

   FLOWLOGIC = 0x36;
   FLOWEQ0CTL = 0x00;
   FLOWEQ1CTL = 0x02;
   FLOWSTB = 0xD0;
   FLOWSTBEDGE = 0x03;
   GPIFHOLDAMOUNT = 0x01;

   EP8FIFOCFG = 0x0D;
   IFCONFIG = 0xc6;
}

void initUdmaWrite()
{
   FLOWLOGIC = 0x70;
   FLOWEQ0CTL = 0x00;
   FLOWEQ1CTL = 0x08;		// Enable CTL3 signal for XRO
   FLOWSTB = 0x11;
   FLOWSTBEDGE = 0x03;
   if(udmaMode == TRANSFER_MODE_UDMA2)
	   FLOWSTBHPERIOD = 0x04;  // for UDMA33
   else  
	   FLOWSTBHPERIOD = 0x02;  // for UDMA66
   GPIFHOLDAMOUNT = 0x01;
   EP2GPIFFLGSEL = 0x01;
   IFCONFIG = 0x86;
}

// Write a single byte to the given disk register or buffer
void writePIO8(char addr, WORD indata)
{

   // make sure GPIF is not busy
    while (!gpifIdle())
        {
        }

   GPIFWFSELECT = 0;    // PIO write is waveform 0

   // Write the address/chip selects
    OUTATAPI = addr | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);

    // trigger the GPIF
    XGPIFSGLDATH = MSB(indata);                       // Single bus transaction on the GPIF
    XGPIFSGLDATLX = LSB(indata);                   // Single bus transaction on the GPIF

   // make sure GPIF is not busy
    while (!gpifIdle())
        {
        }

    // Clear the address/chip selects
    OUTATAPI = ATAPI_IDLE_VALUE;

}   

// Write the string to the given disk register or buffer
void writePIO16(char addr, WORD count)
{
    int timeout = IORDY_TIMEOUT_RELOAD;

    GPIFABORT = 0x00;

    while (!gpifIdle())
      ;

   GPIFWFSELECT = 0;    // PIO write is waveform 0

   // Write the address/chip selects
    OUTATAPI = addr | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);


   // set up GPIF transfer
   EP2GPIFTCH = MSB(count >> 1);
   EP2GPIFTCL = LSB(count >> 1);  // # of GPIF wordwide transactions
   EP2GPIFTRIG = 0x00;           // trigger the transfer
//   GPIFTRIG = 0x00;        // GPIFTRIG[2] = RD/WR BIT (1 = READ)
                              // xxxxxxxx
                              // ||||||00 b0/b1:  EP bit code:  00=EP2, 01=EP4, 10=EP6, 11=EP8
                              // |||||0-- b3:     R/W#: W=0, R=1
                              // 0------- b7:     DONE
    while (!gpifIdle())
      ; 
}   

// Read the status register.  Added to save space.
BYTE readATAPI_STATUS_REG()
{
   return(readPIO8(ATAPI_STATUS_REG));
}

// Read a string from the given disk register or buffer
BYTE readPIO8(char addr)
{
   BYTE retval;

   while (!gpifIdle());

   GPIFWFSELECT = (1 << 4);    // PIO read is waveform 1

   // put out address of interest
   OUTATAPI = addr | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);

   // trigger the GPIF
   retval = XGPIFSGLDATLX;         // Single bus transaction on the GPIF

   while (!gpifIdle());             // wait till GPIF is done before getting real data

   retval = XGPIFSGLDATLNOX;        // get data from last GPIF transaction
   OUTATAPI = ATAPI_IDLE_VALUE;     // Clear the address/chip selects      

    return(retval);
}

WORD readWordPIO8(char addr)
{
   WORD retval;
   
   while (!gpifIdle());

   GPIFWFSELECT = (1 << 4);    // PIO read is waveform 1

   // put out address of interest
   OUTATAPI = addr | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);

   // trigger the GPIF
   retval = XGPIFSGLDATLX;         // Single bus transaction on the GPIF

   while (!gpifIdle());             // wait till GPIF is done before getting real data
 
   retval = (XGPIFSGLDATLNOX << 8) + XGPIFSGLDATH;        // get data from last GPIF transaction
   OUTATAPI = ATAPI_IDLE_VALUE;     // Clear the address/chip selects      

   return(retval);
}


// Read a string from the given disk register or buffer
void readPIO16(char addr, WORD count)
{
   // check for GPIF ready
   while (!gpifIdle());

   GPIFWFSELECT = (1);    // PIO read is waveform 1

   // Write the address/chip selects
   OUTATAPI = addr | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);

   // set up for GPIF transfer - wordwide, so count/2
   EP8GPIFTCH = MSB(count >> 1);     
   EP8GPIFTCL = LSB(count >> 1);

   // trigger GPIF.  No longer wait 'til done
   GPIFTRIG = 0x07;        // GPIFTRIG[2] = RD/WR BIT (1 = READ)
                           // GPIFTRIG[1..0] = EP#, 00=ep2, 01=ep4, 10 = ep6, 11=ep8

}   

// read count WORDs using UDMA
void readUDMA(DWORD count)
{
   // check for GPIF ready
   while (!gpifIdle());

   GPIFWFSELECT = (3);    // UDMA read is waveform 3

   // Write the address/chip selects -- Note that this is not the same register as the ATAPI_DATA_REG
   OUTATAPI = CS(3) | DA(0) | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);

   // set up for GPIF transfer - wordwide
   GPIFTCB3 = 0;
   GPIFTCB2 = ((BYTE *) &count)[1];
   GPIFTCB1 = ((BYTE *) &count)[2];
   GPIFTCB0 = LSB(count);

   FLOWSTATE = 0x82;

   // trigger GPIF and wait till done
   GPIFTRIG = 0x07;        // GPIFTRIG[2] = RD/WR BIT (1 = READ)
                           // GPIFTRIG[1..0] = EP#, 00=ep2, 01=ep4, 10 = ep6, 11=ep8

   // Wait for the drive interrupt.
   while(!(IOA & 0x01));

   if (!gpifIdle())
   {
      abortGPIF();
   }

   FLOWSTATE = 0x00;
}   

// Wait for all of the bulk buffers to be full (or all of the data to be received)
// Switch to manual mode
// Write count WORDs using UDMA
// Return drive status
void writeUDMA(DWORD count)
{
   BYTE drvstat=0;
   WORD byteCount;
   BYTE i;

   // Special code for switching between auto/manual modes.  Make sure that all of
   // the buffers are full before switching.
   for (i = 0, byteCount = 0; i < 4 && byteCount < dataTransferLen; i++, byteCount +=wPacketSize)
   {
      // wait for the sector to show up
      while ((EP2CS & bmEPEMPTY))
         ;

      // commit the buffer(s) to the GPIF
      EP2BCL = 0x00;
      WRITEDELAY();
   }

   initUdmaWrite();

   EP2FIFOCFG = bmAUTOOUT | bmWORDWIDE;

   // check for GPIF ready
   while (!gpifIdle());

   GPIFWFSELECT = (2 << 2);    // UDMA write is waveform 2

   // Write the address/chip selects -- Note that this is not the same register as the ATAPI_DATA_REG
   OUTATAPI = CS(3) | DA(0) | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);

   // set up for GPIF transfer - wordwide
   GPIFTCB3 = 0;
   GPIFTCB2 = ((BYTE *) &count)[1];
   GPIFTCB1 = ((BYTE *) &count)[2];
   GPIFTCB0 = LSB(count);

   FLOWSTATE = 0x83;

   // trigger GPIF and wait till done
   EP2GPIFTRIG = 0;

   // Wait for the drive interrupt.
   while( !((drvstat = IOA) & 0x01))
      ;

   if (!gpifIdle())
   {
      abortGPIF();
   }

   FLOWSTATE = 0x00;

   // cancel AUTO OUT mode
   EP2FIFOCFG = bmWORDWIDE;
   WRITEDELAY();
}   

void abortGPIF()
{
   FLOWSTATE = 0x00;  // xro - take out of UDMA flowstate
   GPIFABORT = 0xff;

   // reset the transaction count state machine,  in FX2 revs up to and
   // including Rev D, there is a bug that prevents the GPIF state machine
   // from properly reseting following an abort.  The following code is a
   // workaround for this problem.  See the FX2 chip errata for details.
#ifdef GPIF_ABORT_BUG_PRESENT
   mymemmovexx(&GPIF_WAVE_DATA, (BYTE xdata *) AbortWave, sizeof(WaveDataPio4));

   GPIFWFSELECT = 0;    // PIO write is waveform 0

   EP2GPIFTCH = 0;     
   EP2GPIFTCL = 1;
   GPIFSGLDATLX = 0;

   mymemmovexx(&GPIF_WAVE_DATA, (BYTE xdata *) WaveDataPio4, sizeof(WaveDataPio4));
#endif
}

