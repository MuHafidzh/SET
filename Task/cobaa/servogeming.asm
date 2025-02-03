$NOMOD51
$INCLUDE (8051.MCU)

      org   0000h

mov tmod,#01h ; Set Timer 0 to Mode 1
Start:
      
      call KANAN
      call Delay
      call KIRI
      call Delay
      jmp Start
KANAN:
      mov th0,#0FFh ; Set high byte for Timer 0 (adjusted for -90 degrees)
      mov tl0,#0A0h ; Set low byte for Timer 0 (adjusted for -90 degrees)
      clr tf0       ; Clear overflow flag
      setb tr0      ; Start Timer 0
      cpl p1.0      ; Complement P1.0
      jnb tf0,$     ; Wait for overflow
      clr tf0       ; Clear overflow flag
      clr tr0       ; Stop Timer 0
 
KIRI: 
      mov th0,#0FFh ; Set high byte for Timer 0 (adjusted for 90 degrees)
      mov tl0,#0D0h ; Set low byte for Timer 0 (adjusted for 90 degrees)
      clr tf0       ; Clear overflow flag
      setb tr0      ; Start Timer 0
      cpl p1.0      ; Complement P1.0
      jnb tf0,$     ; Wait for overflow
      clr tf0       ; Clear overflow flag
      clr tr0       ; Stop Timer 0
      
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