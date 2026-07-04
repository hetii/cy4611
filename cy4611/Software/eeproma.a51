NAME      EEPROMa

public      EEWaitForStop
extrn	    CODE(EEStartAndAddr)

$include (fx2regs.inc)

EEPROMSEG	SEGMENT	CODE

	RSEG	EEPROMSEG
	USING	0
EEWaitForStop:
        MOV     DPTR,#I2CS
        MOVX    A,@DPTR
        JB      ACC.6,EEWaitForStop
	RET


public WaitForEEPROMWrite
; Keep writing the EEPROM address until the device ACKs
; This means that the write is complete.
WaitForEEPROMWrite:
	LCALL   EEWaitForStop
AckLoop:
	LCALL   EEStartAndAddr

waitForBusy:
	; Wait for BUSY to be 0
        MOV     DPTR,#0E678H
        MOVX    A,@DPTR
        JNB     ACC.0,waitForBusy
	
	; Set the STOP bit
        MOVX    A,@DPTR
        ORL     A,#040H
        MOVX    @DPTR,A
	LCALL   EEWaitForStop

        MOVX    A,@DPTR
        JNB     ACC.1,AckLoop		; If no ACK, try again
        RET     

      end
