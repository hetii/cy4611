//-----------------------------------------------------------------------------
//   File:      fw.c
//   Contents:   Firmware frameworks task dispatcher and device request parser
//            source.
//
// indent 3.  NO TABS!
//
//   Copyright (c) 2001 Cypress Semiconductor
//
// $Archive: /USB/atapifx2/software/fw.c $
// $Date: 1/15/02 10:12a $
// $Revision: 45 $
//-----------------------------------------------------------------------------
#include "fx2.h"
#include "fx2regs.h"
#include "gpif.h"
#include "atapi.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define DELAY_COUNT   0x9248*8L      // Delay for 8 sec at 24Mhz, 4 sec at 48
// USB constants
// Class specific setup commands
#define SC_BOMS_RESET           (0x21)      // Hard/soft depends on wValue field 0 = hard

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
BOOL      Rwuen;
BOOL      Selfpwr;
volatile BOOL   Sleep;                  // Sleep mode enable flag
BYTE   AlternateSetting;   // Alternate settings
BYTE   Configuration;      // Current configuration


//WORD   pDeviceDscr;   // Pointer to Device Descriptor; Descriptors may be moved
//WORD   pDeviceQualDscr;
//WORD   pHighSpeedConfigDscr;
//WORD   pFullSpeedConfigDscr;   
WORD   pConfigDscr;
WORD   pOtherConfigDscr;   
//WORD   pStringDscr;

BYTE intrfcSubClass;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void SetupCommand(void);
void TD_Init(void);
void TD_Poll(void);
//BOOL TD_Suspend(void);
//BOOL TD_Resume(void);
void DisconAndWaitForVbus();

WORD wPacketSize;


//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------

// Task dispatcher

void main(void)
{
   BYTE     i;

   // Init globals
   MaxPIO = 0;                // reset MaxPIO value
   Sleep = FALSE;             // Reset suspend flag
   Rwuen = FALSE;             // Disable remote wakeup
   Selfpwr = FALSE;           // Disable self powered

   // if we are hung up in a GPIF transfer, abort it.  How could this happen?  If
   // we ended up here after a USB Reset or a MSC Reset, it is possible that the
   // GPIF is hung waiting for a transfer that will never complete.
   abortGPIF(); // TPM: Unconditionally abort

   TD_Init();

   // The following section of code is used to relocate the descriptor table from
   // ROM to RAM.  It is done here because we are done with the halfKBuffer at this point.
   // Although it looks wierd, the only way to get the proper values for the offsets
   // is to tell the compiler that we want the address of the variable, not the variable itself.
   mymemmovexx(halfKBuffer, (char xdata *) &DeviceDscr, (WORD)&DscrEndOffset);
//   pDeviceDscr = (WORD)(halfKBuffer + (WORD)&DeviceDscrOffset);
//   pDeviceQualDscr = (WORD)(halfKBuffer + (WORD)&DeviceQualDscrOffset);
//   pHighSpeedConfigDscr = (WORD)(halfKBuffer + (WORD)&HighSpeedConfigDscrOffset);
//   pFullSpeedConfigDscr = (WORD)(halfKBuffer + (WORD)&FullSpeedConfigDscrOffset);
//   pStringDscr = (WORD)(halfKBuffer + (WORD)&StringDscrOffset);

   halfKBuffer[(WORD) &IntrfcSubClassHighSpeedOffset] = 
      halfKBuffer[(WORD) &IntrfcSubClassFullSpeedOffset] = intrfcSubClass;


   for (i = 0; i < ATAPI_INQUIRY_SERIAL_LEN *2; i++)
      halfKBuffer[i+(WORD)&SerialNumberOffset] = localSerialNumber[i];
  
   EZUSB_IRQ_ENABLE();            // Enable USB interrupt (INT2)
   EZUSB_ENABLE_RSMIRQ();            // Wake-up interrupt

   INTSETUP |= (bmAV2EN);     // Enable INT 2 autovectoring

   USBIE |= bmSUDAV | bmSUSP | bmURES | bmHSGRANT;   // Enable selected interrupts
   EA = 1;                  // Enable 8051 interrupts

   // Renumerate if necessary.  Do this by checking the renum bit.  If it
   // is already set, there is no need to renumerate.  The renum bit will
   // already be set if this firmware was loaded from an eeprom or if we
   // have already been through this code once and we are here again
   // because of a USB Reset.
   if(!(USBCS & bmRENUM))
   {
       EZUSB_Discon(TRUE);   // renumerate
   }

#ifdef VBUS_DETECT
   // check for the presence of VBus and re-connect.  If we loaded from eeprom
   // we are disconnected and need to connect.  If we just renumerated this
   // is not necessary but doesn't hurt anything.  VBus on the Rev *B 4611 boards
   // is tied to port A.6.
   if (IOA & bmBIT6)
   {
      USBCS &=~bmDISCON;
   }
   else
   {
      DisconAndWaitForVbus();
   }
#else
   // unconditionally re-connect.  If we loaded from eeprom we are
   // disconnected and need to connect.  If we just renumerated this
   // is not necessary but doesn't hurt anything
   USBCS &=~bmDISCON;
#endif

   CKCON = (CKCON&(~bmSTRETCH)) | FW_STRETCH_VALUE; // Set stretch to 0 (after renumeration)

   // complete the handshake phase of any pending SETUP transfer.
   // The only time this should happen is after a MSC Reset.  We want
   // to go through all of the EP init code before handshaking the status
   // phase of the MSC Reset.
   EP0CS |= bmHSNAK;

   // Task Dispatcher
   while(TRUE)                     // Main Loop
   {
#ifdef VBUS_DETECT
      // we need to constantly monitor VBus (tied to Port A.6) and disconnect if
      // it isn't there.  This is to avoid driving D+ (a spec violation) when the
      // host isn't there.
      if (!(IOA & bmBIT6))
         DisconAndWaitForVbus();
#endif

      TD_Poll();
   }
}

#ifdef VBUS_DETECT
// This function monitors VBus from the host and stays disconnected until VBus is
// is present.  VBus is tied to Port A.6 on the new FX2 ATAPI tailgate boards.
// (4611 Rev *B)
void DisconAndWaitForVbus()
{
   USBCS |= bmDISCON;
   while (!(IOA & bmBIT6));
   USBCS &= ~bmDISCON;
}
#endif
   

// Device request parser
void SetupCommand(void)
{
   void   *dscr_ptr;

   switch(SETUPDAT[1])
   {
      case SC_GET_DESCRIPTOR:                      // *** Get Descriptor
         switch(SETUPDAT[3])         
            {
               case GD_DEVICE:                     // Device
                  SUDPTRH = MSB(pDeviceDscr);
                  SUDPTRL = LSB(pDeviceDscr);
                  break;
               case GD_DEVICE_QUALIFIER:            // Device Qualifier
                  SUDPTRH = MSB(pDeviceQualDscr);
                  SUDPTRL = LSB(pDeviceQualDscr);
                  break;
               case GD_CONFIGURATION:              // Configuration
                  SUDPTRH = MSB(pConfigDscr);
                  SUDPTRL = LSB(pConfigDscr);
                  break;
               case GD_OTHER_SPEED_CONFIGURATION:  // Other Speed Configuration
                  // fx2bug - need to support multi other configs
                  SUDPTRH = MSB(pOtherConfigDscr);
                  SUDPTRL = LSB(pOtherConfigDscr);
                  break;
               case GD_STRING:            // String
                  if(dscr_ptr = (void *)EZUSB_GetStringDscr(SETUPDAT[2]))
                  {
                     SUDPTRH = MSB(dscr_ptr);
                     SUDPTRL = LSB(dscr_ptr);
                  }
                  else 
                     EZUSB_STALL_EP0();   // Stall End Point 0
                  break;
               default:                   // Invalid request
                  EZUSB_STALL_EP0();      // Stall End Point 0
            }  // switch
         break;
      case SC_GET_INTERFACE:                  // *** Get Interface
         EP0BUF[0] = AlternateSetting;
         EP0BCH = 0;
         EP0BCL = 1;
         break;
      case SC_SET_INTERFACE:                  // *** Set Interface
         AlternateSetting = SETUPDAT[2];
         break;
      case SC_SET_CONFIGURATION:               // *** Set Configuration
      // BUGBUG -- Add delay for the CATC
         Configuration = SETUPDAT[2];
         break;
      case SC_GET_CONFIGURATION:               // *** Get Configuration
         EP0BUF[0] = Configuration;
         EP0BCH = 0;
         EP0BCL = 1;
         break;
      case SC_GET_STATUS:                 // *** Get Status
            switch(SETUPDAT[0])
            {
               case GS_DEVICE:            // Device
                  EP0BUF[0] = ((BYTE)Rwuen << 1) | (BYTE)Selfpwr;
                  EP0BUF[1] = 0;
                  EP0BCH = 0;
                  EP0BCL = 2;
                  break;
               case GS_INTERFACE:         // Interface
                  EP0BUF[0] = 0;
                  EP0BUF[1] = 0;
                  EP0BCH = 0;
                  EP0BCL = 2;
                  break;
               case GS_ENDPOINT:         // End Point
                  if (SETUPDAT[4] == 0x2)
                  {
                     EP0BUF[0] = (EP2CS & bmEPSTALL);
                  }
                  else if (SETUPDAT[4] == 0x88)
                  {
                     EP0BUF[0] = (EP8CS & bmEPSTALL);
                  }
                  else
                  {
                     EZUSB_STALL_EP0();   // Stall End Point 0
                     break;
                  }

                  EP0BUF[1] = 0;
                  EP0BCH = 0;
                  EP0BCL = 2;
                  break;
               default:                   // Invalid Command
                  EZUSB_STALL_EP0();      // Stall End Point 0
            }
         break;
      case SC_CLEAR_FEATURE:                  // *** Clear Feature
            switch(SETUPDAT[0])
            {
               case FT_DEVICE:            // Device
                  if(SETUPDAT[2] == 1)
                     Rwuen = FALSE;       // Disable Remote Wakeup
                  else
                     EZUSB_STALL_EP0();   // Stall End Point 0
                  break;
               case FT_ENDPOINT:          // End Point
                  if(SETUPDAT[2] == 0)
                  {
                     if (SETUPDAT[4] == 0x2)
                     {
                        ResetAndArmEp2();
                        TOGCTL = 0x2;
                        TOGCTL = 0x22;       // reset data toggle
                        EP2CS = 0;           // Clear stall bit
                     }
                     else if (SETUPDAT[4] == 0x88)
                        {
                        TOGCTL = 0x18;
                        TOGCTL = 0x38;       // reset data toggle
                        EP8CS = 0;           // Clear stall bit
                        }
                     else
                        EZUSB_STALL_EP0();   // Stall End Point 0
                     }
                  else
                     EZUSB_STALL_EP0();      // Stall End Point 0
                  break;
            }  
         break;
      case SC_SET_FEATURE:                  // *** Set Feature
            switch(SETUPDAT[0])
            {
               case FT_DEVICE:            // Device
                  if(SETUPDAT[2] == 1)
                     Rwuen = TRUE;        // Enable Remote Wakeup
                  else if(SETUPDAT[2] == 2)
                     // Set Feature Test Mode.  The core handles this request.  However, it is
                     // necessary for the firmware to complete the handshake phase of the
                     // control transfer before the chip will enter test mode.  It is also
                     // necessary for FX2 to be physically disconnected (D+ and D-)
                     // from the host before it will enter test mode.
                     break;
                  else
                     EZUSB_STALL_EP0();   // Stall End Point 0
                  break;
               case FT_ENDPOINT:          // End Point
                  if(SETUPDAT[2] == 0)
                     {
                     if (SETUPDAT[4] == 0x2)
                        EP2CS = bmEPSTALL;         // Set stall bit
                     else if (SETUPDAT[4] == 0x88)
                        EP8CS = bmEPSTALL;         // Set stall bit
                     else
                        EZUSB_STALL_EP0();   // Stall End Point 0
                     }
                  else
                     EZUSB_STALL_EP0();   // Stall End Point 0
                  break;
            }  
         break;

      // This is a first attempt at completing a mass storage reset.  It is not yet tested.
      case SC_MASS_STORAGE_RESET:
         // Verify that the command is actually a MS reset command sent to the proper interface
         if (SETUPDAT[0] == 0x21 && SETUPDAT[4] == 0)  // Our interface number is hard coded (0) in DSCR.A51
         {
            // All we really need to do in response to a MSC Reset is restart using
            // a soft reset (jump to 0x00).  This will re-initialize the drive and
            // endpoints.
            EZUSB_IRQ_CLEAR();
            INT2CLR = bmSUDAV;         // Clear SUDAV IRQ

            // force a soft reset after the iret.
            EA = 0;
            softReset();
         }
         else
            EZUSB_STALL_EP0();   // Stall End Point 0
         break;

      default:                            // *** Invalid Command
            EZUSB_STALL_EP0();            // Stall End Point 0
   }

   // Acknowledge handshake phase of device request
   // Required for rev C does not effect rev B
    EP0CS |= bmHSNAK;
}

// Wake-up interrupt handler
void resume_isr(void) interrupt WKUP_VECT
{
   EZUSB_CLEAR_RSMIRQ();
}

STRINGDSCR xdata *   EZUSB_GetStringDscr(BYTE StrIdx)
{
   extern BYTE code StringDscr0;
   extern BYTE code StringDscr1;
   extern BYTE code StringDscr2;
   extern BYTE code StringDscr3;

   switch (StrIdx)
   {
      case 0:
         return((STRINGDSCR xdata *)&StringDscr0);
      case 1:
         return((STRINGDSCR xdata *)&StringDscr1);
      case 2:
         return((STRINGDSCR xdata *)&StringDscr2);
      case 3:
         return((STRINGDSCR xdata *)&StringDscr3);
      default:
         return(0);
   }
}
