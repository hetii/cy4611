//-----------------------------------------------------------------------------
//   File:      eeprom.c
//   Contents:   EEPROM update firmware source.  (Write only)
//
//   indent 3.  NO TABS!
//
//   Copyright (c) 2002 Cypress Semiconductor
//
// $Archive: /USB/atapifx2/software/eeprom.c $
// $Date: 1/21/02 2:36p $
// $Revision: 3 $
//-----------------------------------------------------------------------------
#include "fx2.h"
#include "fx2regs.h"
#include "atapi.h"

#define EEPROM_ADDR 0x51
#define EEPROM_PAGE_SIZE 16

extern void    EEWaitForStop();
static void EEStartAndAddr();
extern void WaitForEEPROMWrite();

///////////////////////////////////////////////////////////////////////////////////////

// Write the entire EEPROM
void EEPROMWrite(WORD len)
{
   WORD i;
   WORD addr;

   for (addr = 0; addr < len; addr += wPacketSize)
      {
      // Wait for host to send data
      while(EP2CS & bmEPEMPTY)       
         ;

      // Write the data from the USB buffer to the EEPROM
      for (i = 0; i < wPacketSize; i+= EEPROM_PAGE_SIZE)
         {
         EEPROMWritePage(addr+i, EP2FIFOBUF+i);
         }

      // Give up the buffer
      EP2BCL = 0x80;     
      }
}
// Write one page of data to the EEPROM.
void EEPROMWritePage(WORD addr, BYTE xdata * ptr)
{
   BYTE		i;
   BYTE 	 	ee_str[EEPROM_PAGE_SIZE+2];

	ee_str[0] = MSB(addr);
	ee_str[1] = LSB(addr);
	for (i = 2; i < EEPROM_PAGE_SIZE+2; i++)
   	{
    	ee_str[i] = ptr[i-2];
   	}

   // Make sure the i2c interface is idle
   EEWaitForStop();
   
   // Write the device address   
   EEStartAndAddr();
   for (i = 0; i < EEPROM_PAGE_SIZE + 2; i++)
   {
      while (!(I2CS & 1))  // Poll the done bit
         ;
      I2DAT = ee_str[i];
   }	
   while (!(I2CS & 1))  // Poll the done bit
      ;
   I2CS |= bmSTOP;
   WaitForEEPROMWrite();
}

void EEStartAndAddr()
{
      I2CS |= bmSTART;
      I2DAT = EEPROM_ADDR << 1;
}
