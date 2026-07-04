public    	softReset

?STACK		SEGMENT   IDATA
softResetSeg	SEGMENT	CODE

	RSEG	softResetSeg
    USING	0
softReset:
	mov		a, #0
	push 	acc
	push 	acc
	reti
    end
    
