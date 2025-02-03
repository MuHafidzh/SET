$NOMOD51
$INCLUDE (8051.MCU)

      org   0000h

timer: 
      ; Initialize Timer 0 in mode 1 (16-bit timer)
      mov tmod, #01h
      
      ; Load initial values for Timer 0
      mov th0, #0FEh
      mov tl0, #00Bh
      
      ; Clear Timer 0 overflow flag and start Timer 0
      clr tf0
      setb tr0
      
      ; Toggle P1.1
      cpl p1.1
      
      ; Wait for Timer 0 to overflow
wait_overflow:
      jnb tf0, wait_overflow
      
      ; Clear Timer 0 overflow flag and stop Timer 0
      clr tf0
      clr tr0
      
      ; Toggle P1.1 again
      cpl p1.1
      
      ; Implement delay using nested loops
      mov R2, #4
delay_loop3:
      mov R1, #255
delay_loop2:
      mov R0, #255
delay_loop1:
      dec R0
      nop
      cjne R0, #00, delay_loop1
      dec R1
      cjne R1, #00, delay_loop2
      dec R2
      cjne R2, #00, delay_loop3
      
      ; Reload Timer 0 with new values
      mov th0, #0F8h
      mov tl0, #02Fh
      
      ; Clear Timer 0 overflow flag and start Timer 0
      clr tf0
      setb tr0
      
      ; Toggle P1.1
      cpl p1.1
      
      ; Wait for Timer 0 to overflow
wait_overflow2:
      jnb tf0, wait_overflow2
      
      ; Clear Timer 0 overflow flag and stop Timer 0
      clr tf0
      clr tr0
      
      ; Repeat the process
      jmp timer

END