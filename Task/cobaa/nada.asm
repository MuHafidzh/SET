org   0000h
      jmp   Start
      
      org   0bh
      jmp   interrupt_timer_0
      
      org   1bh
      jmp   interrupt_timer_1

      org   03h
      jmp   interrupt_external_0

      org   13h
      jmp   interrupt_external_1

      org   0100h
Start:	
      mov ie,#10001111b ; Enable external interrupts
      mov tmod,#011h
      mov R1, #191 ; Initial value for delay register
      
      mov dptr,#NotesTable
      
Loop:	
      clr a
      movc a,@a+dptr
      jz EndOfTable
      mov R4,a
      inc dptr
      clr a
      movc a,@a+dptr
      mov R5,a
      inc dptr
      mov tl0,R4
      mov th0,R5
      clr tf0
      setb tr0
      call Delay
      jmp Loop

EndOfTable:
      clr tr0 ; Stop timer 0
      clr p3.0 ; Turn off speaker
      call Delay
      mov dptr,#NotesTable ; Reset pointer to start of table
      jmp Loop

NotesTable:
      db 043h, 0fch ; DO
      db 0ach, 0fch ; RE
      db 09h, 0fdh ; MI
      db 033h, 0fdh ; FA
      db 082h, 0fdh ; SOL
      db 0c7h, 0fdh ; LA
      db 05h, 0feh ; SI
      db 021h, 0feh ; DO*
      db 01h, 01h ; berenti sejenak
      db 021h, 0feh ; DO*
      db 05h, 0feh ; SI
      db 0c7h, 0fdh ; LA
      db 082h, 0fdh ; SOL
      db 033h, 0fdh ; FA
      db 09h, 0fdh  ; MI
      db 0ach, 0fch ; RE
      db 043h, 0fch ; DO
      db 00h, 00h ;End of table

      
interrupt_timer_0:
      clr tf0
      mov tl0,R4
      mov th0,R5
      cpl p3.0
      setb tr0
      reti

interrupt_timer_1:
      clr tf1
      mov tl1,#0afh
      mov th1,#03ch
      setb tr1
      reti

interrupt_external_0:
      inc R1
      cjne R1, #255, ext0_end
      mov R1, #255
ext0_end:
      reti

interrupt_external_1:
      dec R1
      cjne R1, #127, ext1_end
      mov R1, #127
ext1_end:
      reti

Delay:
      mov A, R1 ; Copy R1 to A
      mov R2, A ; Copy A to R2 for delay loop
Loop2:
      mov R0, #255
Loop1:
      dec R0
      nop ;di jeda satu siklus mesin
      cjne R0, #00, Loop1
      dec R2
      cjne R2, #00, Loop2
      ret ; return

;====================================================================
      END