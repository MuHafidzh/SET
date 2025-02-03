$NOMOD51
$INCLUDE (8051.MCU)
      org   0000h
Start:	
      mov tmod,#001h
      mov th0,#0FEh
      mov tl0,#00Bh
      clr tf0
      setb tr0
      cpl p1.1
      jnb tf0,$
      clr tf0
      clr tr0
      
Loop:	
      cpl p1.1
      call delay
      jmp Loop
      
delay:
      mov R2,#16
Loop3:
      mov R1,#255
Loop2:
      mov R0,#255
Loop1:
      dec R0
      nop
      cjne R0,#00,Loop1
      dec R1
      cjne R1,#00,Loop2
      dec R2
      cjne R2,#00,Loop3
      
      
      mov th0,#0F8h
      mov tl0,#02Fh 
      clr tf0
      setb tr0
      cpl p1.1
      jnb tf0,$
      clr tf0
      clr tr0
      jmp Start
      
END