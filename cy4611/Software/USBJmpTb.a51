NAME      USBJmpTbl

extrn code (ISR_Sudav, ISR_Susp, ISR_Ures, ISR_Highspeed)
;            ISR_Ep0ack, ISR_Stub, ISR_Ep0in, ISR_Ep0out, ISR_Ep1in, ISR_Ep1out,\
;            ISR_Ep2inout, ISR_Ep4inout, ISR_Ep6inout, ISR_Ep8inout,ISR_Ibn)

;extrn code (ISR_Ep0pingnak, ISR_Ep1pingnak, ISR_Ep2pingnak, ISR_Ep4pingnak,\
;            ISR_Ep6pingnak, ISR_Ep8pingnak, ISR_Errorlimit, ISR_Ep2piderror,\
;            ISR_Ep4piderror, ISR_Ep6piderror, ISR_Ep8piderror, ISR_Ep2pflag)

;extrn code (ISR_Ep4pflag, ISR_Ep6pflag, ISR_Ep8pflag, ISR_Ep2eflag, ISR_Ep4eflag,\
;            ISR_Ep6eflag, ISR_Ep8eflag, ISR_Ep2fflag, ISR_Ep4fflag, ISR_Ep6fflag,\
;            ISR_Ep8fflag, ISR_GpifComplete, ISR_GpifWaveform)

;extrn code (ISR_GpifComplete, ISR_Stub, ISR_GpifWaveform)

public      USB_Int2AutoVector, USB_Jump_Table
;------------------------------------------------------------------------------
; Interrupt Vectors
;------------------------------------------------------------------------------
      CSEG   AT 23H		; UART0 Vector - placeholder to keep the monitor
      ds 3			; from squashing our code

      CSEG   AT 3BH		; UART1 Vector - placeholder to keep the monitor
      ds 3			; from squashing our code

      CSEG   AT 43H
USB_Int2AutoVector   equ   $ + 2
      ljmp   USB_Jump_Table   ; Autovector will replace byte 45

      CSEG   AT 53H		; INT4 Vector - placeholder to keep the int4
      ds 3			; autovector from squashing our code

;------------------------------------------------------------------------------
; USB Jump Table
;------------------------------------------------------------------------------
;?PR?USB_JUMP_TABLE?USBJT   segment   code page   ; Place jump table on a page boundary
      CSEG    AT 1000h	;; must be xx00 aligned autovector jump table
USB_Jump_Table:   
      ljmp  ISR_Sudav            ;(00) Setup Data Available
      db   0
;      ljmp  ISR_Sof              ;(04) Start of Frame
;      db   0
;      ljmp  ISR_Sutok            ;(08) Setup Data Loading
;      db   0
      CSEG  AT 100ch
      ljmp  ISR_Susp             ;(0C) Global Suspend
      db    0
      ljmp  ISR_Ures             ;(10) USB Reset     
      db   0
      ljmp  ISR_Highspeed        ;(14) Entered High Speed
      db   0
;      ljmp  ISR_Ep0ack             ;(18) ISR_Ep0ack:  EP0ACK
;      db   0
;      ljmp  ISR_Stub             ;(1C) Reserved
;      db   0
;      ljmp  ISR_Stub             ;(20) ISR_Ep0in:  EP0 In
;      db   0
;      ljmp  ISR_Stub             ;(24) ISR_Ep0out:  EP0 Out
;      db   0
;      ljmp  ISR_Stub             ;(28) ISR_Ep1in:  EP1 In
 ;     db   0
;      ljmp  ISR_Stub             ;(2C) ISR_Ep1out:  EP1 Out
;      db   0
;      ljmp  ISR_Stub         	 ;(30) ISR_Ep2inout:  EP2 In/Out
;      db   0
;      ljmp  ISR_Stub         	 ;(34) ISR_Ep4inout:  EP4 In/Out
;      db   0
;      ljmp  ISR_Stub         	 ;(38) ISR_Ep6inout:  EP6 In/Out
;      db   0
;      ljmp  ISR_Stub         	 ;(3C) ISR_Ep8inout:  EP8 In/Out
;      db   0
;      ljmp  ISR_Stub             ;(40) ISR_Ibn:  IBN
;      db   0
;      ljmp  ISR_Stub             ;(44) Reserved
;      db   0
;      ljmp  ISR_Stub       	 ;(48) ISR_Ep0pingnak:  EP0 PING NAK
;      db   0
;      ljmp  ISR_Stub       	 ;(4C) ISR_Ep1pingnak:  EP1 PING NAK
;      db   0
;      ljmp  ISR_Stub       	 ;(50) ISR_Ep2pingnak:  EP2 PING NAK
;      db   0
;      ljmp  ISR_Stub       	 ;(54) ISR_Ep4pingnak:  EP4 PING NAK
;      db   0
;      ljmp  ISR_Stub       	 ;(58) ISR_Ep6pingnak:  EP6 PING NAK
;      db   0
;      ljmp  ISR_Stub       	 ;(5C) ISR_Ep8pingnak:  EP8 PING NAK
;      db   0
;      ljmp  ISR_Stub	         ;(60) ISR_Errorlimit:  Error Limit
;      db   0
;      ljmp  ISR_Stub             ;(64) Reserved
;      db   0
;      ljmp  ISR_Stub             ;(68) Reserved
;      db   0
;      ljmp  ISR_Stub             ;(6C) Reserved
;      db   0
;      ljmp  ISR_Stub		 ;(70) ISR_Ep2piderror:  EP2 ISO Pid Sequence Error
;      db   0
;      ljmp  ISR_Stub      	 ;(74) ISR_Ep4piderror:  EP4 ISO Pid Sequence Error
;      db   0
;      ljmp  ISR_Stub 		 ;(78) ISR_Ep6piderror:  EP6 ISO Pid Sequence Error
;      db   0
;      ljmp  ISR_Stub		 ;(7C) ISR_Ep8piderror:  EP8 ISO Pid Sequence Error
;      db   0
;INT4_Jump_Table
;      ljmp  ISR_Stub	          ;(80) ISR_Ep2pflag:  EP2 Programmable Flag
;      db   0
;      ljmp  ISR_Stub	          ;(84) ISR_Ep4pflag:  EP4 Programmable Flag
;      db   0
;      ljmp  ISR_Stub	          ;(88) ISR_Ep6pflag:  EP6 Programmable Flag
;      db   0
;      ljmp  ISR_Stub	         ;(8C) ISR_Ep8pflag:  EP8 Programmable Flag
;      db   0
;      ljmp  ISR_Stub	         ;(90) ISR_Ep2eflag:  EP2 Empty Flag
;      db   0
;      ljmp  ISR_Stub	         ;(94) ISR_Ep4eflag:  EP4 Empty Flag
;      db   0
;      ljmp  ISR_Stub	         ;(98) ISR_Ep6eflag:  EP6 Empty Flag
;      db   0
;      ljmp  ISR_Stub	         ;(9C) ISR_Ep8eflag:  EP8 Empty Flag
;      db   0
;      ljmp  ISR_Stub	         ;(A0) ISR_Ep2fflag:  EP2 Full Flag
;      db   0
;      ljmp  ISR_Stub	         ;(A4) ISR_Ep4fflag:  EP4 Full Flag
;      db   0
;      ljmp  ISR_Stub	         ;(A8) ISR_Ep6fflag:  EP6 Full Flag
;      db   0
;      ljmp  ISR_Stub	         ;(AC) ISR_Ep8fflag:  EP8 Full Flag
;      db   0
;      ljmp  ISR_GpifComplete     ;(B0) GPIF Operation Complete
;      db   0
;      ljmp  ISR_GpifWaveform	 ;(B4) ISR_GpifWaveform:  GPIF Waveform
;      db   0
;
      end
