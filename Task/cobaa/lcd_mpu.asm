;====================================================================
; main.asm file generated by new project wizard
;
; created:   thu apr 25 2024
; processor: 80c31
; compiler:  asem-51 (proteus)
;====================================================================

$nomod51
$include (8051.mcu)

;====================================================================
; definitions
;====================================================================

;====================================================================
; variables
;====================================================================

;====================================================================
; reset and interrupt vectors
;====================================================================

      ; reset vector
      org   0000h
      jmp   start

;====================================================================
; code segment
;====================================================================

      org   0100h

start: 
    lcall i2c_init
    call lcd_init
    lcall i2c_start	
    mov a, #11010000b
    acall i2c_write
    mov a, #6bh
    acall i2c_write
    mov a, #00h
    acall i2c_write
    acall i2c_stop 

    lcall i2c_init
    lcall i2c_start
    mov a, #11010000b
    acall i2c_write
    mov a, #1bh
    acall i2c_write
    mov a, #00h
    acall i2c_write
    acall i2c_stop
      

loop:
    ; Gyro X
    lcall i2c_init
    lcall i2c_start
    mov a, #11010000b
    acall i2c_write
    mov a, #43h
    acall i2c_write
    lcall i2c_start
    mov a, #11010001b
    acall i2c_write
    acall i2c_read
    mov r0, a
    acall i2c_nack
    acall i2c_stop

    mov a, #80h
    call lcd_set_cursor

    mov a, #'G'
    call lcd_send_data
    mov a, #'y'
    call lcd_send_data
    mov a, #'r'
    call lcd_send_data
    mov a, #'o'
    call lcd_send_data
    mov a, #'X'
    call lcd_send_data
    mov a, #':'
    call lcd_send_data

    mov a, r0
    call lcd_print

    ; Accel X
    lcall i2c_init
    lcall i2c_start
    mov a, #11010000b
    acall i2c_write
    mov a, #3bh
    acall i2c_write
    lcall i2c_start
    mov a, #11010001b
    acall i2c_write
    acall i2c_read
    mov r0, a
    acall i2c_nack
    acall i2c_stop

    mov a, #0C0h
    call lcd_set_cursor

    mov a, #'A'
    call lcd_send_data
    mov a, #'c'
    call lcd_send_data
    mov a, #'c'
    call lcd_send_data
    mov a, #'e'
    call lcd_send_data
    mov a, #'l'
    call lcd_send_data
    mov a, #'X'
    call lcd_send_data
    mov a, #':'
    call lcd_send_data

    mov a, r0
    call lcd_print
    
    call delay
    
    jmp loop

i2c_init:
    setb p1.2
    setb p1.3
    ret

i2c_reset:
    clr p1.3
    setb p1.2
    setb p1.3
    clr p1.2
    ret

i2c_start:
    setb p1.3
    clr p1.2
    clr p1.3
    ret

i2c_stop:
    clr p1.3
    clr p1.2
    setb p1.3
    setb p1.2
    ret

i2c_write:
    mov r7, #08
i2c_write_loop:
    clr p1.3
    rlc a
    mov p1.2, c
    setb p1.3
    djnz r7, i2c_write_loop
    clr p1.3
    setb p1.2
    setb p1.3
    mov c, p1.2
    clr p1.3
    ret

i2c_ack:
    clr p1.2
    setb p1.3
    clr p1.3
    setb p1.2
    ret

i2c_nack:
    setb p1.2
    setb p1.3
    clr p1.3
    setb p1.3
    ret

i2c_read:
    mov r7, #08
i2c_read_loop:
    clr p1.3
    setb p1.3
    mov c, p1.2
    rlc a
    djnz r7, i2c_read_loop
    clr p1.3
    setb p1.2
    ret    

lcd_init:
    mov a, #00111000b
    call send_inst
    mov a, #00000110b
    call send_inst
    mov a, #00001100b
    call send_inst
    mov a, #00000001b
    call send_inst 
    ret

send_inst:
    clr p1.0
    setb p1.1
    mov p2, a
    call delay
    clr p1.1
    setb p1.1
    call delay
    ret	

lcd_set_cursor:
    clr p1.0
    setb p1.1
    mov p2, a
    call delay
    clr p1.1
    setb p1.1
    call delay
    ret

lcd_send_data:
    setb p1.0
    setb p1.1
    mov p2, a
    call delay
    clr p1.1
    setb p1.1
    call delay
    ret  

lcd_print:
    mov b, #64h
    div ab
    mov r1, a
    mov a, b
    
    mov b, #0ah
    div ab
    mov r2, a
    mov r3, b
    
    mov a, r1
    add a, #30h
    mov r1, a
    
    mov a, r2
    add a, #30h
    mov r2, a
    
    mov a, r3
    add a, #30h
    mov r3, a
    
    mov a, r1
    call lcd_send_data
    
    mov a, r2
    call lcd_send_data
    
    mov a, r3
    call lcd_send_data
    
    ret    

delay:
    mov r7, #01
delay_loop1:
    mov r6, #055
delay_loop2:
    mov r5, #055
delay_loop3:
    dec r5
    nop
    cjne r5, #00, delay_loop3
    dec r6
    cjne r6, #00, delay_loop2
    dec r7
    cjne r7, #00, delay_loop1
    ret
    
 
;====================================================================
      end