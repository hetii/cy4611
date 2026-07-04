//-----------------------------------------------------------------------------
//   File:      atapi.h
//   Contents:   Header file
//
// Copyright (c) 1999 Cypress Semiconductor, Inc. All rights reserved
//
// $Archive: /USB/atapifx2/software/atapi.h $
// $Date: 1/23/02 9:31a $
// $Revision: 44 $
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// Interface specific function definitions
///////////////////////////////////////////////////////////////////////////////
void hardwareReset();
void writePIO8(char addr, WORD indata);
void writePIO16(char addr, WORD count);
void readPIO16(char addr, WORD count);
BYTE readPIO8(char addr);
BYTE readATAPI_STATUS_REG(void);
WORD readWordPIO8(char addr);
void initUdmaRead();
void initUdmaWrite();
void readUDMA(DWORD count);
void writeUDMA(DWORD count);
void abortGPIF();
bit waitForIntrq();
void initUSB(void);
void LoadandVerifyWaveForm(BYTE *WaveForm);
WORD getDriveDataLen();
void ResetAndArmEp2();
void configureATATransferMode(BYTE mode);
void EEPROMWritePage(WORD addr, BYTE xdata * ptr);
void EEPROMWrite(WORD len);
void waitForInBuffer();
#define FAIL_IN_ON_TIMEOUT 1
#define DONT_FAIL_IN_ON_TIMEOUT 0
//#define FX2_56PIN_PACKAGE 1         // need to compile code for internal space
//#define DEVICE_TYPE_IS_IDE 0
//#define DEVICE_TYPE_IS_SCSI 1

///////////////////////////////////////////////////////////////////////////////
// Common function definitions
///////////////////////////////////////////////////////////////////////////////

//------------ ide ------------------------------
void softReset();
void TD_Poll(void);
void SetupCommand(void);
void stallEP2OUT();
BYTE SCSITestUnitReady();
void resetATADevice();
void resetATAPIDevice();
void processCBW();
void SCSIInquiryToATAPI();
void sendUSBS(BYTE passOrFail);
void sendRAMData(BYTE xdata *inData, WORD dataTransferLen);
BYTE generalSCSIInCommand();
BYTE generalSCSIOutCommand();
bit generalIDEInCommand();
bit generalIDEOutCommand();
bit sendSCSICommand(char xdata *cmdbuf);
void failedIn();
void ATAPIIdDevice();
WORD readPIO16toXdata(char addr, char xdata *inbuffer, WORD count, bit ignoreDriveLen);
bit waitForBusyBit();
BYTE inDataFromDrive();
void prepareForIDECommand();
void mymemmovexx(BYTE xdata * dest, BYTE xdata * src, WORD len);
bit checkForMedia();

#define IGNORE_DRIVE_LEN 1
#define LISTEN_TO_DRIVE_LEN 0
#define BUFFER_SIZE 0x200
#define SCSIInquiryData_ ((BYTE xdata *) SCSIInquiryData)

extern void serialNumber();
extern code char IntrfcSubClassHighSpeed;
extern code char IntrfcSubClassFullSpeed;
extern DWORD dataTransferLen;
extern DWORD driveCapacity;
extern WORD wPacketSize;
extern char udmaMode;      // global to keep track of which udma mode we are in

extern WORD DeviceDscrOffset;
extern WORD DeviceQualDscrOffset;
extern WORD HighSpeedConfigDscrOffset;
extern WORD FullSpeedConfigDscrOffset;
extern WORD StringDscrOffset;
extern WORD UserDscrOffset;
extern WORD IntrfcSubClassHighSpeedOffset;
extern WORD IntrfcSubClassFullSpeedOffset;
extern WORD SerialNumberOffset;
extern WORD DscrEndOffset;
extern bit bShortPacketSent;
extern bit bExtAddrSupport;

extern xdata BYTE halfKBuffer[BUFFER_SIZE];

extern xdata BYTE SCSIInquiryData[44];
extern bit scsi;
extern char code *sensePtr;
extern const char code senseInvalidFieldInCDB[];
extern const char code senseOk[];
extern const char code WaveDataPio4[128];
extern const char code WaveDataPio0[128];
extern BYTE intrfcSubClass;
extern BYTE udmaErrorCount;

// support for higher PIO modes
extern char MaxPIO;   
#define PIO4            0x02
#define PIO3            0x01
#define PIO_MODE1        0x09
#define PIO_MODE2        0x0A
#define PIO_MODE3        0x0B
#define PIO_MODE4        0x0C

#define dataTransferLenMSW	((WORD *) (&dataTransferLen))[0]
#define dataTransferLenLSW	((WORD *) (&dataTransferLen))[1]
#define dataTransferLenMSB	((BYTE *) (&dataTransferLen))[0]
#define dataTransferLen2SB	((BYTE *) (&dataTransferLen))[1]
#define dataTransferLen3SB	((BYTE *) (&dataTransferLen))[2]
#define dataTransferLenLSB	((BYTE *) (&dataTransferLen))[3]

#define USBS_PASSED      0
#define USBS_FAILED      1
#define USBS_PHASE_ERROR 2

// Local defines from the mass storage class spec
#define SC_MASS_STORAGE_RESET       0xff
#define CBW_TAG                     4
#define CBW_DATA_TRANSFER_LEN_LSB   8
#define CBW_DATA_TRANSFER_LEN_MSB   9
#define CBW_FLAGS                   12
#define CBW_FLAGS_DIR_BIT           0x80
#define CBW_LUN                     13
#define CBW_CBW_LEN                 14
#define CBW_CBW_LEN_MASK            0x1f
#define CBW_DATA_START              15

#define USB_MS_RBC_SUBCLASS               1
#define USB_MS_CD_ROM_SUBCLASS            2
#define USB_MS_SCSI_TRANSPARENT_SUBCLASS  6

// Local defines for the mass storage device
#define PROCESS_CBW_TIMEOUT_RELOAD  0x7000

#define HS_BULK_PACKET_SIZE     0x200
#define FS_BULK_PACKET_SIZE     0x40

#define   min(a,b) (((a)<(b))?(a):(b))
#define   max(a,b) (((a)>(b))?(a):(b))

#define PIO_ADDR_COMMAND   7
#define PIO_ADDR_DATA      0

#define ATAPI_INTRQ (PINSA & 0x01)


#define ATAPI_STATUS_BUSY_BIT    0x80
#define ATAPI_STATUS_DRDY_BIT    0x40
#define ATAPI_STATUS_DF_BIT      0x20
#define ATAPI_STATUS_DSC_BIT     0x10
#define ATAPI_STATUS_DRQ_BIT     0x08
#define ATAPI_STATUS_CORR_BIT    0x04
#define ATAPI_STATUS_INDEX_BIT   0x02
#define ATAPI_STATUS_ERROR_BIT   0x01

// Errors for WRITE DMA
#define ATAPI_ERROR_ICRC_BIT	      0x80
#define ATAPI_ERROR_WP_BIT		      0x40
#define ATAPI_ERROR_MEDIA_CHANGED_BIT 0x20
#define ATAPI_ERROR_ABRT_BIT	      0x04
#define ATAPI_ERROR_NO_MEDIA_BIT      0x02

#define ATA_SECTOR_SIZE             0x200

#define IDE_COMMAND_ID_DEVICE                0xec
#define IDE_ID_TOTAL_SECTORS_LSW             60*2
#define IDE_ID_TOTAL_SECTORS_MSW             61*2
#define IDE_ID_TOTAL_48_BIT_SECTORS_LSW      100*2
#define IDE_ID_TOTAL_48_BIT_SECTORS_2SW      101*2
#define IDE_ID_TOTAL_48_BIT_SECTORS_3SW      102*2
#define IDE_ID_TOTAL_48_BIT_SECTORS_MSW      103*2
#define ATAPI_INQUIRY_SERIAL                 10*2
#define ATAPI_INQUIRY_SERIAL_LEN             12
#define ATAPI_COMMAND_ID_DEVICE              0xa1
#define ATAPI_COMMAND_ATAPI_PACKET           0xa0
#define ATAPI_COMMAND_SOFT_RESET             0x08
#define ATAPI_COMMAND_CHECK_POWER            0xe5
#define ATAPI_COMMAND_EXEC_DIAG              0x90
#define ATAPI_COMMAND_IDLE                   0xe3
#define ATAPI_COMMAND_IDLE_IMMED             0xe1
#define ATAPI_COMMAND_NOP                    0x00
#define ATAPI_COMMAND_SERVICE                0xa2
#define ATAPI_COMMAND_SET_FEATURES           0xef
#define ATAPI_COMMAND_SLEEP                  0xe6
#define ATAPI_COMMAND_STANDBY                0xe2
#define ATAPI_COMMAND_STANDBY_IMMED          0xe0

#define ATAPI_PACKET_LOAD_UNLOAD             0xa6

#define ATAPI_CONTROL_REG_SOFT_RESET   4
#define ATAPI_CONTROL_REG_DEFAULT      8

// Converted RBC commands
#define ATA_COMMAND_READ_10               0x20 
#define ATA_COMMAND_READ_10_EXT           0x24 
#define ATA_COMMAND_WRITE_10              0x30 
#define ATA_COMMAND_WRITE_10_EXT          0x34 
#define ATA_COMMAND_VERIFY_10             0x40 
#define ATA_COMMAND_VERIFY_10_EXT         0x42
#define ATA_COMMAND_DMAREAD_RETRY         0xC8
#define ATA_COMMAND_DMAREAD_RETRY_EXT     0x25
// #define ATA_COMMAND_DMAREAD_NORETRY       0xC9  Obsolete in ATA-6
#define ATA_COMMAND_DMAWRITE_RETRY        0xCA 
#define ATA_COMMAND_DMAWRITE_RETRY_EXT    0x35 
// #define ATA_COMMAND_DMAWRITE_NORETRY      0xCB  Obsolete in ATA-6

// Still unconverted RBC commands
#define ATA_COMMAND_INQUIRY                        0x12 
#define ATA_COMMAND_MODE_SELECT_6                  0x15 
#define ATA_COMMAND_MODE_SENSE_6                   0x1A  
#define ATA_COMMAND_PREVENT_ALLOW_MEDIUM_REMOVAL   0x1E 
#define ATA_COMMAND_START_STOP_UNIT                0x1B 
#define ATA_COMMAND_TEST_UNIT_READY                0x00  
#define ATA_COMMAND_WRITE_BUFFER                   0x3B  

// Optional RBC commands
#define ATA_COMMAND_NOP                      0x00
#define ATA_COMMAND_FORMAT_UNIT              0x04 
#define ATA_COMMAND_PERSISTENT_RESERVE_IN    0x5E 
#define ATA_COMMAND_PERSISTENT_RESERVE_OUT   0x5F 
#define ATA_COMMAND_RELEASE_6                0x17  
#define ATA_COMMAND_REQUEST_SENSE            0x03  
#define ATA_COMMAND_RESERVE_6                0x16  
#define ATA_COMMAND_SYNCHRONIZE_CACHE        0x35 

// Fields in the INQUIRY
#define SCSI_INQUIRY_DEVICE_CLASS      0
#define SCSI_INQUIRY_REMOVABLE_BIT     0x80
#define SCSI_INQUIRY_REMOVABLE_BYTE    1
#define ATAPI_INQUIRY_REMOVABLE_BYTE   0
#define SCSI_INQUIRY_DATA_FORMAT       3
#define SCSI_INQUIRY_MANUFACTURER      8
#define ATAPI_INQUIRY_MANUFACTURER     27
#define SCSI_INQUIRY_MANUFACTURER_LEN  24
#define ATAPI_INQUIRY_REVISION         73
#define SCSI_INQUIRY_REVISION          32
#define SCSI_INQUIRY_REVISION_LEN      4

// Fields in the structure returned by the IDENTIFY DEVICE (ECh) and
// IDENTIFY PACKET DEVICE (A1h) commands (BYTE offsets)
#define IDENTIFY_FIELD_VALIDITY        53*2
#define IDENTIFY_ADVANCED_PIO          64*2
#define IDENTIFY_48BIT_ADDRESSING      83*2
#define IDENTIFY_UDMA_MODES            88*2

#define IDENTIFY_NUM_CYLINDERS_LSB     54*2
#define IDENTIFY_NUM_CYLINDERS_MSB     IDENTIFY_NUM_CYLINDERS_LSB+1
#define IDENTIFY_NUM_HEADS             55*2
#define IDENTIFY_NUM_SECT_PER_TRACK    56*2

// UDMA supported modes bits from IDENTIFY DEVICE
#define UDMA_MODE0      0x01
#define UDMA_MODE1      0x02
#define UDMA_MODE2      0x04
#define UDMA_MODE3      0x08
#define UDMA_MODE4      0x10
#define UDMA_MODE5      0x20

// Transfer mode settings
#define TRANSFER_MODE_DEFAULT    0x00
#define TRANSFER_MODE_PIO1       0x09
#define TRANSFER_MODE_PIO2       0x0A
#define TRANSFER_MODE_PIO3       0x0B
#define TRANSFER_MODE_PIO4       0x0C
#define TRANSFER_MODE_UDMA0      0x40
#define TRANSFER_MODE_UDMA1      0x41
#define TRANSFER_MODE_UDMA2      0x42
#define TRANSFER_MODE_UDMA3      0x43
#define TRANSFER_MODE_UDMA4      0x44
#define TRANSFER_MODE_UDMA5      0x45

// Local ATAPI defines -- Command register block
#define ATAPI_DATA_REG        (DA(0)|CS(2))
#define ATAPI_ERROR_REG       (DA(1)|CS(2))
#define ATAPI_FEATURE_REG     (DA(1)|CS(2))
#define ATAPI_INT_CAUSE_REG   (DA(2)|CS(2))
#define ATAPI_BYTE_COUNT_LSB  (DA(4)|CS(2))
#define ATAPI_BYTE_COUNT_MSB  (DA(5)|CS(2))
#define ATAPI_DRIVESEL_REG    (DA(6)|CS(2))
#define ATAPI_STATUS_REG      (DA(7)|CS(2))
#define ATAPI_COMMAND_REG     (DA(7)|CS(2))
#define ATAPI_NULL_REG        (DA(7)|CS(3))

// Local ATAPI defines -- Control register block
#define ATAPI_ALT_STATUS_REG  (DA(6)|CS(1))
#define ATAPI_CONTROL_REG     (DA(6)|CS(1))

// IDE registers -- Overlay on the ATAPI register space
#define ATA_DATA_REG          (DA(0)|CS(2))
#define ATA_ERROR_REG         (DA(1)|CS(2))
#define ATA_SECTOR_COUNT_REG  (DA(2)|CS(2))
#define ATA_LBA_LSB_REG       (DA(3)|CS(2))
#define ATA_LBA_2SB_REG       (DA(4)|CS(2))
#define ATA_LBA_MSB_REG       (DA(5)|CS(2))
#define ATA_DRIVESEL_REG      (DA(6)|CS(2))
#define ATA_COMMAND_REG       (DA(7)|CS(2))

#define   pDeviceDscr ((WORD)(halfKBuffer + (WORD)&DeviceDscrOffset))
#define   pDeviceQualDscr ((WORD)(halfKBuffer + (WORD)&DeviceQualDscrOffset))
#define   pHighSpeedConfigDscr ((WORD)(halfKBuffer + (WORD)&HighSpeedConfigDscrOffset))
#define   pFullSpeedConfigDscr ((WORD)(halfKBuffer + (WORD)&FullSpeedConfigDscrOffset))
#define   pStringDscr ((WORD)(halfKBuffer + (WORD)&StringDscrOffset))
//#define   WaveDataPio0 (WaveDataPio4+4)
extern idata char localSerialNumber[ATAPI_INQUIRY_SERIAL_LEN*2];
