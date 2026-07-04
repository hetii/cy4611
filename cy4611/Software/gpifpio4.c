//                                                                                         
// This program configures the General Purpose Interface (GPIF) for Fx2.                   
// Parts of this program are automatically generated using the GPIF Tool.                  
// Please do not modify sections of text which are marked as "DO NOT EDIT ...".            
// You can modify the comments section of this GPIF program file using the dropdown menus  
// and pop-up dialogs. These controls are available as hot spots in the text. Modifying the
// comments section will generate program code which will implement your GPIF program.     
//                                                                                         
// DO NOT EDIT ...                                                                         
// GPIF Initialization                                                                     
// Interface Timing      Async                                                             
// Internal Ready Init   IntRdy=1                                                          
// CTL Out Tristate-able Tristate                                                          
// SingleWrite WF Select     2                                                             
// SingleRead WF Select      3                                                             
// FifoWrite WF Select       0                                                             
// FifoRead WF Select        1                                                             
// Data Bus Idle Drive   Tristate                                                          
// END DO NOT EDIT                                                                         
                                                                                           
// DO NOT EDIT ...                                                                         
// GPIF Wave Names                                                                         
// Wave 0   = PIO4WR                                                                       
// Wave 1   = PIO4RD                                                                       
// Wave 2   = UNUSED                                                                       
// Wave 3   = UNUSED                                                                       
                                                                                           
// GPIF Ctrl Outputs   Level                                                               
// CTL 0    = DIOW     CMOS                                                                
// CTL 1    = DIOR     CMOS                                                                
// CTL 2    = DMACK    CMOS                                                                
// CTL 3    = CTL 3    CMOS                                                                
// CTL 4    = CTL 4    CMOS                                                                
// CTL 5    = CTL 5    CMOS                                                                
                                                                                           
// GPIF Rdy Inputs                                                                         
// RDY0     = IORDY                                                                        
// RDY1     = RESERVED                                                                     
// RDY2     = DMARQ                                                                        
// RDY3     = FLAGD                                                                        
// RDY4     = RDY4                                                                         
// RDY5     = RDY5                                                                         
// FIFOFlag = FIFOFlag                                                                     
// IntReady = IntReady                                                                     
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 0: PIO4WR                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode Activate  Activate  Activate  Activate  NO Data   NO Data   NO Data            
// NextData SameData  SameData  SameData  NextData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    No Int             
// IF/Wait  Wait 4    IF        Wait 1    IF        Wait 256  Wait 256  Wait 256           
//   Term A           IORDY               FIFOFlag                                         
//   LFunc            AND                 AND                                              
//   Term B           IORDY               FIFOFlag                                         
// Branch1            Then 2              ThenIdle                                         
// Branch0            Else 1              ElseIdle                                         
// DIOW         0         0         1         1         z         z         z         z    
// DIOR         1         1         1         1         z         z         z         z    
// DMACK        1         1         1         1         z         z         z         z    
// CTL 3        1         1         1         1         z         z         z         z    
// CTL 4                                                                                   
// CTL 5                                                                                   
//                                                                                         
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 1: PIO4RD                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode NO Data   NO Data   Activate  NO Data   NO Data   NO Data   NO Data            
// NextData SameData  SameData  NextData  SameData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    No Int             
// IF/Wait  Wait 4    IF        IF        Wait 256  Wait 256  Wait 256  Wait 256           
//   Term A           IORDY     IORDY                                                      
//   LFunc            AND       AND                                                        
//   Term B           IORDY     IORDY                                                      
// Branch1            Then 2    ThenIdle                                                   
// Branch0            Else 1    ElseIdle                                                   
// DIOW         1         1         1         z         z         z         z         z    
// DIOR         0         0         1         z         z         z         z         z    
// DMACK        1         1         1         z         z         z         z         z    
// CTL 3        1         1         1         z         z         z         z         z    
// CTL 4                                                                                   
// CTL 5                                                                                   
//                                                                                         
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 2: UNUSED                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode NO Data   Activate  NO Data   NO Data   NO Data   NO Data   NO Data            
// NextData SameData  NextData  SameData  SameData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    No Int             
// IF/Wait  Wait 1    Wait 10   Wait 6    IF        Wait 256  Wait 256  Wait 256           
//   Term A                               FIFOFlag                                         
//   LFunc                                AND                                              
//   Term B                               FIFOFlag                                         
// Branch1                                ThenIdle                                         
// Branch0                                ElseIdle                                         
// DIOW         1         0         1         1         z         z         z         z    
// DIOR         1         1         1         1         z         z         z         z    
// DMACK        1         1         1         1         z         z         z         z    
// CTL 3        z         z         z         z         z         z         z         z    
// CTL 4                                                                                   
// CTL 5                                                                                   
//                                                                                         
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 3: UNUSED                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode NO Data   NO Data   Activate  NO Data   NO Data   NO Data   NO Data            
// NextData SameData  SameData  SameData  SameData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    No Int             
// IF/Wait  Wait 1    Wait 10   Wait 6    IF        Wait 256  Wait 256  Wait 256           
//   Term A                               FIFOFlag                                         
//   LFunc                                AND                                              
//   Term B                               FIFOFlag                                         
// Branch1                                ThenIdle                                         
// Branch0                                ElseIdle                                         
// DIOW         1         1         1         1         z         z         z         z    
// DIOR         1         0         1         1         z         z         z         z    
// DMACK        1         1         1         1         z         z         z         z    
// CTL 3        z         1         z         z         z         z         z         z    
// CTL 4                                                                                   
// CTL 5                                                                                   
//                                                                                         
// END DO NOT EDIT                                                                         
                                                                                           
// GPIF Program Code                                                                       
                                                                                           
// DO NOT EDIT ...                                                                         
#include "fx2.h"                                                                           
#include "fx2regs.h"                                                                       
// END DO NOT EDIT                                                                         
                                                                                           
// DO NOT EDIT ...                                                                         
const char xdata WaveData[128] =                                                           
{                                                                                          
// Wave 0                                                                                  
/* LenBr */ 0x04,     0x11,     0x01,     0x3F,     0x00,     0x00,     0x00,     0x00,    
/* Opcode*/ 0x02,     0x03,     0x02,     0x07,     0x00,     0x00,     0x00,     0x00,    
/* Output*/ 0xFE,     0xFE,     0xFF,     0xFF,     0x00,     0x00,     0x00,     0x00,    
/* LFun  */ 0x00,     0x00,     0x00,     0x36,     0x00,     0x00,     0x00,     0x00,    
// Wave 1                                                                                  
/* LenBr */ 0x04,     0x11,     0x3f,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Opcode*/ 0x00,     0x01,     0x07,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Output*/ 0xFD,     0xFD,     0xFF,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,    
// Wave 2  UDMA Write
/* LenBr */ 0x08,     0x01,     0x06,     0x1C,     0x01,     0x01,     0x02,     0x07,    
/* Opcode*/ 0x01,     0x00,     0x02,     0x03,     0x02,     0x02,     0x26,     0x00,    
/* Output*/ 0xFF,     0xFB,     0xFA,     0xFA,     0xF8,     0xFB,     0xFB,     0xFF,    
/* LFun  */ 0x09,     0x09,     0x12,     0x09,     0x00,     0x2d,     0x36,     0x3F,     
// Wave 3  UDMA Read
/* LenBr */ 0x08,     0x01,     0x13,     0x01,     0x01,     0x02,     0x01,     0x07,    
/* Opcode*/ 0x01,     0x00,     0x01,     0x00,     0x00,     0x26,     0x26,     0x00,    
/* Output*/ 0xFF,     0xFB,     0xF8,     0xFB,     0xFB,     0xFB,     0xFF,     0xFF,    
/* LFun  */ 0x09,     0x09,     0x09,     0x1b,     0x00,     0x2D,     0x36,     0x3F, 
};                                                                                         
// END DO NOT EDIT                                                                         
                                                                                           
// DO NOT EDIT ...                                                                         
const char xdata InitData[7] =                                                             
{                                                                                          
/* Regs  */ 0x80,0x80,0x00,0x00,0x06,0xB1,0x11              
};                                                                                         
// END DO NOT EDIT                                                                         
                                                                                           

void GpifInit( void )
{
  BYTE xdata *Source;
  BYTE xdata *Dest;
  BYTE x;

//  GPIFABORT = 0;                      // abort any pending operation
//GPIFREADY = InitData[0];              // name changed to GPIFREADYCFG in Fx2
  GPIFCTLCFG = InitData[1];
  GPIFIDLECS = InitData[2];
  GPIFIDLECTL = InitData[3];
  IFCONFIG = (IFCONFIG & 0xF8) | 0x06;  // Set IFCFG to 0x06 - GSTATE & GPIF 
//IFCONFIG = InitData[4];               // Bus 16 - No longer present in Fx2
  GPIFWFSELECT = InitData[5];

  Source = &WaveData[127];              // Transfer the GPIF Tool generated data
  Dest = 0xE47F;


  for (x = 0; x < 128; x++)
  {
    *Dest-- = *Source--;
  }

//	INT4SETUP = INT4SFC | INT4_INTERNAL; // setup INT4 as internal source
//	GENIE = 0x01;                 // Enable GPIF interrupt 
//	EIEX4 = 0x01;
}
// END DO NOT EDIT                                                                         

// TO DO: You may add additional code below.
//

#define TMOUT 0x0020    // Default Timeout TODO: Set this appropriately

void OtherInit( void )
{

// interface initialization
  CPUCS = 0x02;   // don't drive CLKOUT (CLKOE=1=>Not Driving CLKOUT) 
  IFCONFIG = 0xC6;// use internal FCLK -> FCLKSRC=1 1100 0010
			      // at 48MHz -> 3048MHZ=1
			      // don't enable FCLK output -> FCLKOE=0
			      // don't invert FCLK output polarity -> FCLPOL=0
			      // sync to FCLK -> ASYNC=0
			      // output GPIF states -> GSTATE=1
			      // use GPIF as master -> IFCFG[1..0]=10
  GPIFREADYCFG |= 0xC0;   // INTRDY=1, SAS=1 (sync), RDY5CFG=0 (Fx2 only)
  GPIFABORT= 0x00;

  PORTCCFG = 0xFF;         // use Port C as alternate function GPIFADR[7:0]		
  PORTECFG |= 0x80;        // use PE7 as alternate function GPIFADR8

  EP2FIFOCFG = 0x05;	// use 16-bit databus -> WORDWIDE=1
  EP4FIFOCFG = 0x05;	// commit zero length pkt via ext.
                        // ... PKTEND pin only -> ZEROLENIN=1
  EP6FIFOCFG = 0x05;	// 8051 decides if to commit data to
                        // ... OUT FIFO -> AUTOOUT=0
  EP8FIFOCFG = 0x05;	// 8051 dispatches an IN pkt by writing
                        // ... byte count -> AUTOIN=0
			      // normal active flags -> INFM1=0
			      // normal active flags -> OEP1=0

  EP1OUTCFG = 0xA0;	// ep1out is valid BULK OUT 64
  EP1INCFG = 0xA0;	// ep1in is valid BULK IN 64

  // default: all endpoints have their VALID bit set
  // default: TYPE1 = 1 and TYPE0 = 0 --> BULK  
  // default: EP2 and EP4 DIR bits are 0 (OUT direction)
  // default: EP6 and EP8 DIR bits are 1 (IN direction)
  // default: EP2, EP4, EP6, and EP8 are double buffered

  // we are just using the default values, yes this is not necessary...
  EP2CFG = 0xA2;     // ep2 is valid BULK OUT 512 double buffered
  EP4CFG = 0xA0;	   // ep4 is valid BULK OUT 512 double buffered
  EP6CFG = 0xE2;	   // ep6 is valid BULK IN 512 double buffered
  EP8CFG = 0xE0;	   // ep8 is valid BULK IN 512 double buffered

  // The PF is highly configurable...
  EP2FIFOPFH = 0x00;	   // PF=0 when BC > PF -> Decis=0 (1 byte in FIFO)
  EP2FIFOPFL = 0x00;	   // PF and BC refer to the current pkt -> PKTSTAT=0

  EP4FIFOPFH = 0x00;	   // PF=0 when BC > PF -> Decis=0 (1 byte in FIFO)
  EP4FIFOPFL = 0x00;	   // PF and BC refer to the current pkt -> PKTSTAT=0

  EP6FIFOPFH = 0x40;	   // PF=0 when BC <= PF -> Decis=0 (1 byte in FIFO)		
  EP6FIFOPFL = 0x3F;	   // PF and BC refer to the current pkt -> PKTSTAT=1

  EP8FIFOPFH = 0x40;    // PF=0 when BC <= PF -> Decis=0 (1 byte in FIFO)
  EP8FIFOPFL = 0x3F;	   // PF and BC refer to the current pkt -> PKTSTAT=1

  EP2AUTOINLENH = 0x00; // EPxPKTLENH/L is for IN's only...
  EP2AUTOINLENL = 0x00;

  EP4AUTOINLENH = 0x00;
  EP4AUTOINLENL = 0x00;

  EP6AUTOINLENH = 0x00;
  EP6AUTOINLENL = 0x40;	// limit EP6IN to 64 bytes for 1.1 speeds

  EP8AUTOINLENH = 0x00;
  EP8AUTOINLENL = 0x40;	// limit EP8IN to 64 bytes for 1.1 speeds

// end interface intialization

  // we are just using the default values, yes this is not necessary...
  EP1OUTCFG = EP1INCFG = 0xA0;

  // out endpoints do not come up armed
  
  // since the defaults are double buffered we must write 
  // ...dummy byte counts twice
  EP2BCL = EP4BCL = 0x80;   // arm EP2OUT & EP4OUT by writing to
                            // ...the byte count w/skip
  EP2BCL = EP4BCL = 0x80;   // arm EP2OUT & EP4OUT by writing to 
                            // ...the byte count w/skip
}

// write byte to PERIPHERAL, using GPIF
bit Peripheral_SingleByteWrite( WORD gaddr, BYTE gdata )
{
  BYTE transaction_err = 0x00;
 
  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;          // setup GPIF address 

  XGPIFSGLDATLX = gdata;                  // initiate GPIF write transaction

  while( !( GPIFIDLECS & 0x80 ) )       // poll GPIFIDLECS.7 Done bit
  {                                  
    if( ++transaction_err > TMOUT )  // trap GPIF transaction for TMOUT
    {                                
      GPIFABORT = 0x01;
      return( 0 );                   // error has occurred
    }
  }

  return( 1 );
}

// write word to PERIPHERAL, using GPIF
bit Peripheral_SingleWordWrite( WORD gaddr, WORD gdata )
{
  BYTE transaction_err = 0x00;
  WORD xdata temp;

  temp = gaddr;
  //temp = 0x0111;
  temp = 0x00BE;


  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;          // setup GPIF address

  // using the register(s) in SFR space
  XGPIFSGLDATH = gdata >> 8;              
  XGPIFSGLDATLX = gdata;                  // initiate GPIF write transaction

  while( !( GPIFTRIG & 0x80 ) )       // poll GPIFIDLECS.7 Done bit
  {                                  
    if( ++transaction_err > TMOUT )  // trap GPIF transaction for TMOUT
    {                                
      GPIFABORT = 0x01;
      return( 0 );                   // error has occurred
    }
  }

  return( 1 );
}

// read byte from PERIPHERAL, using GPIF
bit Peripheral_SingleByteRead( WORD gaddr, BYTE xdata *gdata )
{
  static BYTE g_data = 0x00;
  BYTE transaction_err = 0x00;

  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;          // setup GPIF address

  g_data = XGPIFSGLDATLX;                 // dummy read to initiate GPIF read transaction

  while( !( GPIFIDLECS & 0x80 ) )       // poll GPIFIDLECS.7 Done bit
  {                                  
    if( ++transaction_err > TMOUT )  // trap GPIF transaction for TMOUT
    {                                
      GPIFABORT = 0x01;
      return( 0 );                   // error has occurred
    }
  }

  *gdata = XGPIFSGLDATLNOX;

  return( 1 );
}

// read word from PERIPHERAL, using GPIF
bit Peripheral_SingleWordRead( WORD gaddr, WORD xdata *gdata )
{
  BYTE g_data = 0x00;
  BYTE transaction_err = 0x00;

  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;          // setup GPIF address

  g_data = XGPIFSGLDATLX;                 // dummy read to initiate GPIF read transaction

  while( !( GPIFTRIG & 0x80 ) )       // poll GPIFIDLECS.7 Done bit
  {                                  
    if( ++transaction_err > TMOUT )  // trap GPIF transaction for TMOUT
    {                                
      GPIFABORT = 0x01;
      return( 0 );                   // error has occurred
    }
  }

  *gdata = ( ( WORD )XGPIFSGLDATH << 8 ) | ( WORD )XGPIFSGLDATLNOX;

  return( 1 );
}

// write byte(s) to PERIPHERAL, using GPIF, and EP2FIFO
bit Peripheral_EP2FIFOByteWrite( WORD gaddr, WORD xfrcnt )
{
  BYTE transaction_err = 0x00;

  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;         // setup GPIF address

  EP2GPIFTCH = xfrcnt >> 8;             // setup transaction count
  EP2GPIFTCL = ( BYTE )xfrcnt;

  // replaces above xdata register to SFR mov access
  GPIFTRIG = 0x00;                   // R/W=0, EP[1:0]=00 for EP2 write(s)

  while( !( GPIFTRIG & 0x80 ) )      // poll GPIFIDLECS.7 Done bit
  {                                 // transaction completed
    if( ++transaction_err > TMOUT)  // trap GPIF transaction for TMOUT
    {                                 
      GPIFABORT = 0x01;
      return( 0 );                  // error has occurred
    }
  }

  return( 1 );
}

// write word(s) to PERIPHERAL, using GPIF, and EP2FIFO
bit Peripheral_EP2FIFOWordWrite( WORD gaddr, WORD xfrcnt )
{
  BYTE transaction_err = 0x00;

  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;         // setup GPIF address
  
  EP2GPIFTCH = xfrcnt >> 8;             // setup transaction count
  EP2GPIFTCL = ( BYTE )xfrcnt;
                                    // FIFO -> GPIF transaction(s)
  // replaces above xdata register to SFR mov access
  GPIFTRIG = 0x00;                  // R/W=0, EP[1:0]=00 for EP2 write(s)

  while( !( GPIFTRIG & 0x80 ) )      // poll GPIFIDLECS.7 Done bit
  {                                 // transaction completed
    if( ++transaction_err > TMOUT)  // trap GPIF transaction for TMOUT
    {                                 
      GPIFABORT = 0x01;
      return( 0 );                  // error has occurred
    }
  }

  return( 1 );
}

// read byte(s) from PERIPHERAL, using GPIF, and EP6FIFO
bit Peripheral_EP6FIFOByteRead( WORD gaddr, WORD xfrcnt )
{
  BYTE transaction_err = 0x00;
  BYTE gxfr = 0x00;

  // if GPIF addr is static then this is not necessary, 
  // ...initialize these registers in GPIFInit();
  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;         // setup GPIF address

  // if GPIF transaction count is static then this is not necessary, 
  // ...initialize these registers in GPIFInit();
  EP6GPIFTCH = xfrcnt >> 8;             // setup transaction count
  EP6GPIFTCL = ( BYTE )xfrcnt;

  // replaces above xdata register to SFR mov access
  GPIFTRIG = 0x06;                  // R/W=1, EP[1:0]=10 for EP6 read(s)

  while( !( GPIFTRIG & 0x80 ) )      // poll GPIFIDLECS.7 GPIF Done bit
  {                                   
    if( ++transaction_err > TMOUT ) // trap GPIF transaction for TMOUT
    {                                
      GPIFABORT = 0x01;
      return( 0 );                  // an error has occurred
    }
  }

  // assumes transaction count is buffer size and AUTOIN=0 disabled...
  INPKTEND = 0x06;               // 8051 stobes EP6FIFO packet end
                                    // ... to commit pkt to USB ep6
  return( 1 );
}

// read word(s) from PERIPHERAL, using GPIF, and EP6FIFO
bit Peripheral_EP6FIFOWordRead( WORD gaddr, WORD xfrcnt )
{
  BYTE transaction_err = 0x00;
  BYTE gxfr = 0x00;

  GPIFADRH = gaddr >> 8;
  GPIFADRL = ( BYTE )gaddr;         // setup GPIF address

  EP6GPIFTCH = xfrcnt >> 8;             // setup transaction count
  EP6GPIFTCL = ( BYTE )xfrcnt;

  // replaces above xdata register to SFR mov access
  GPIFTRIG = 0x06;                  // R/W=1, EP[1:0]=10 for EP6 read(s)

  while( !( GPIFTRIG & 0x80 ) )      // poll GPIFIDLECS.7 GPIF Done bit
  {                                   
    if( ++transaction_err > TMOUT ) // trap GPIF transaction for TMOUT
    {                                
      GPIFABORT = 0x01;
      return( 0 );                  // an error has occurred
    }
  }

  // assumes transaction count is buffer size and AUTOIN=0 disabled...
  INPKTEND = 0x06;               // 8051 stobes EP6FIFO packet end
                                    // ... to commit pkt to USB ep6

  return( 1 );
}

void main( void )
{
  WORD xdata wData = 0x0000;
  BYTE xdata bData = 0x00;
  WORD myi = 0x0000;
	
  bit bResult = 1; // bResult will return as 0 if an error has occurred

  OtherInit( );
  GpifInit( );

  if(bResult == 0) // stub out unused functions; avoid "UNCALLED SEGMENT" link error
  {
      bResult = Peripheral_SingleWordWrite( 0x2345, 0x5678 );
      bResult = Peripheral_SingleByteWrite( 0x0055, 0xAA );
      bResult = Peripheral_SingleWordRead( 0x0169, &wData );
      bResult = Peripheral_SingleByteRead( 0x00AA, &bData );
	  bResult = Peripheral_EP2FIFOByteWrite( 0x2355, 0x0040 );
	  bResult = Peripheral_EP2FIFOWordWrite( 0x2355, 0x0040 );
	  bResult = Peripheral_EP6FIFOByteRead( 0x2355, 0x0040 );
	  bResult = Peripheral_EP6FIFOWordRead( 0x2355, 0x0040 );
  }

  while( 1 )
  {
    if( EP2FIFOCFG & 0x01 )  // If 16-bit mode - wordwide
    { // illustrate use of efficient 16 bit functions
      bResult = Peripheral_SingleWordWrite( 0x01FF, 0xFFFF );
      bResult = Peripheral_SingleWordRead( 0x0169, &wData );
    }
    else
    {
      bResult = Peripheral_SingleByteWrite( 0x0055, 0xFF );
      bResult = Peripheral_SingleByteRead( 0x00AA, &bData );
    }
  }

}

