//-----------------------------------------------------------------------------
//   File:      gpif.h
//   Contents:   Header file
//
// Copyright (c) 1999 Cypress Semiconductor, Inc. All rights reserved
//
// $Archive: /USB/atapifx2/software/gpif.h $
// $Date: 6/20/01 7:17a $
// $Revision: 16 $
//-----------------------------------------------------------------------------
#include "atapi.h"

// Mapping of DA and CS numbers to port pins
#define DA(x)  (x << 1)
#define CS(x)  (x << 4)
#define OUTATAPI  IOA

// Other ATAPI signals -- Port A
#define ATAPI_DASP_           (1<<6)         /* input */
#define ATAPI_RESET_          (1<<7)         /* output */
#define GPIF_DISCON_          (1<<2)         /* output */
#define PORTA_OE              0xFE           /* all output except PA.0 */
#define PORTA_CFG_AND         (0xff ^ (DA(7)|CS(3)|ATAPI_RESET_))
#define PORTA_CFG_OR          (bmINT0)
#define ATAPI_IDLE_VALUE      (ATAPI_RESET_ | ATAPI_DASP_ | DA(7)|CS(3))

#define USE_IORDY    1

#undef _at_


//-----------------------------------------------------------------------------
// GPIF Globals
//-----------------------------------------------------------------------------

// Port B is ALL GPIF

#define ATAPI_ADDR_MASK       (DA(7)|CS(3))
#define ATAPI_OE              (DA(7)|CS(3))

#define IORDY_TIMEOUT_RELOAD	    200
#define CLEAR_INTRQ

#define readSectorDataToEP8()                                           \
{                                                                       \
    BYTE i;                                                             \
                                                                        \
   /* check for GPIF ready */                                           \
   while (!(GPIFTRIG & 0x80))    /* SFR space - faster */               \
      ;                                                                 \
   /* PIO read is waveform 1 */                                         \
   GPIFWFSELECT = (1);                                                  \
                                                                        \
                                                                        \
   /* Write the address/chip selects */                                 \
    OUTATAPI = ATAPI_DATA_REG | (~ATAPI_ADDR_MASK & ATAPI_IDLE_VALUE);  \
                                                                        \
   /* trigger GPIF and wait till done if manual mode */                 \
   i = EP8GPIFTRIG;                                                     \
   while (!(GPIFTRIG & 0x80))    /* SFR space - faster */               \
      ;                                                                 \
}

#define gpifIdle() (GPIFTRIG & 0x80)

// States
#define UNCONFIGURED        0
#define WAIT_FOR_CBW        1
#define RECEIVED_OUT_CMD    2
#define RECEIVED_IN_CMD     3


