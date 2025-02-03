$NOMOD51
$INCLUDE (8051.MCU)

      org   0000h

mov tmod,#00h 
Start:
      call KANAN
      call Delay
      call KIRI
      call Delay
      jmp Start
KANAN:
      mov th0,#11000001b
      mov tl0,#00001111b
      clr tf0  ;tf penanda overflow tr penanda dimulai
      setb tr0
      cpl p1.0
      jnb tf0,$
      clr tf0
      clr tr0
 
KIRI: 
      mov th0,#11110000b
      mov tl0,#00001011b
      clr tf0  ;tf penanda overflow tr penanda dimulai
      setb tr0
      cpl p1.0
      jnb tf0,$
      clr tf0
      clr tr0
      
Delay:
      MOV R2, #04
Loop3:
      MOV R1, #255 
Loop2:
      MOV R0, #255
Loop1:
      DEC R0
      NOP ;di jeda satu siklus mesin
      CJNE R0, #00, Loop1
      DEC R1
      CJNE R1, #00, Loop2
      DEC R2
      CJNE R2, #00, Loop3
      RET ; return
      
      

      
 END