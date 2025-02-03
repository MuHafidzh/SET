$nomod51               
$include (80C52.MCU)     

c4_reload_high   equ 0f8h  ; reload value high byte for c4 (261.6 hz)
c4_reload_low    equ 089h  ; reload value low byte for c4 (261.6 hz)

d4_reload_high   equ 0f9h  ; reload value high byte for d4 (293.7 hz)
d4_reload_low    equ 05ah  ; reload value low byte for d4 (293.7 hz)

e4_reload_high   equ 0fah  ; reload value high byte for e4 (329.6 hz)
e4_reload_low    equ 013h  ; reload value low byte for e4 (329.6 hz)

f4_reload_high   equ 0fah  ; reload value high byte for f4 (349.2 hz)
f4_reload_low    equ 068h  ; reload value low byte for f4 (349.2 hz)

g4_reload_high   equ 0fbh  ; reload value high byte for g4 (392 hz)
g4_reload_low    equ 004h  ; reload value low byte for g4 (392 hz)

a4_reload_high   equ 0fbh  ; reload value high byte for a4 (440 hz)
a4_reload_low    equ 090h  ; reload value low byte for a4 (440 hz)

b4_reload_high   equ 0fch  ; reload value high byte for b4 (493.9 hz)
b4_reload_low    equ 00ch  ; reload value low byte for b4 (493.9 hz)

c5_reload_high   equ 0fch  ; reload value high byte for c5 (523.3 hz)
c5_reload_low    equ 045h  ; reload value low byte for c5 (523.3 hz)

tone_delay_base_cnt equ 10d
tone_delay_inc_cnt equ 2d
tone_delay_max_cnt equ 40d
tone_delay_min_cnt equ 1d
tone_revert_cnt equ 8d
tone_revert_cnt_dec equ 8d
tone_first_h_addr equ 30h
tone_first_l_addr equ 31h
tone_last_h_addr equ 3eh
tone_last_l_addr equ 3fh
buzzer_pin equ p2.0
led_pin equ p2.1
th1_reload_high equ 03ch
th1_reload_low equ 0b0h
tmod_data equ 11h

org RESET                  
    jmp start          

org EXTI0
    jmp increment_btn_isr

org EXTI1
    jmp decrement_btn_isr

org TIMER0
    jmp timer0_isr      

org TIMER1         
    jmp timer1_isr 
    
start:
    mov 30h, #c4_reload_high 
    mov 31h, #c4_reload_low 
    mov 32h, #d4_reload_high  
    mov 33h, #d4_reload_low   
    mov 34h, #e4_reload_high  
    mov 35h, #e4_reload_low   
    mov 36h, #f4_reload_high  
    mov 37h, #f4_reload_low   
    mov 38h, #g4_reload_high  
    mov 39h, #g4_reload_low   
    mov 3ah, #a4_reload_high  
    mov 3bh, #a4_reload_low   
    mov 3ch, #b4_reload_high  
    mov 3dh, #b4_reload_low  
    mov 3eh, #c5_reload_high  
    mov 3fh, #c5_reload_low   
    
    mov tmod, #tmod_data      ; both timer to 16-bit
    
     ; timer 0 to generate tone with interrupts
    mov th0, tone_first_h_addr ; load do for the first note 
    mov tl0, tone_first_l_addr ; load do for the first note
    clr tf0
    setb tr0           
    setb et0          
    
    ; timer 1 to generate delay with interrupts, each overflow is 50ms
    mov th1, #th1_reload_high
    mov tl1, #th1_reload_low
    clr tf1
    setb tr1
    setb et1
    
    setb ex1
    setb ex0
    setb it0
    setb it1

    setb ea            ; enable global interrupts
     
     ; r0 and r1 used to store the address of memory for notes
     mov r0, #tone_first_h_addr
     mov r1, #tone_first_l_addr
     
     ; r2 is used to change the tone, changing tone every tone_delay_cnt * 50ms, i.e 40 is 2000ms, loaded from r4
     mov r4, #tone_delay_base_cnt
     mov A, r4
     mov r2, A
     
     ; will revert r0 and r1 to base address (30h and 31h) once r3 gone to zero
     mov r3, #tone_revert_cnt
    
main_loop:
    MOV A, R4
    CJNE A, #tone_delay_max_cnt, check_max
    MOV R4, #tone_delay_max_cnt
check_max:
    SJMP main_loop      ; Infinite loop, everything is handled by interrupts

increment_btn_isr:
    mov A, R4
    add A, #tone_delay_inc_cnt          ; Increment R4 by 10
    jnc no_overflow      ; If no carry, no overflow
    mov A, #tone_delay_max_cnt         ; If overflow, set A to 100
no_overflow:
    mov r4, A
    reti

decrement_btn_isr:
    mov a, r4
    subb a, #tone_delay_inc_cnt         ; Decrement R4 by 10
    jnc no_underflow     ; If no borrow, no underflow
    mov a, #tone_delay_min_cnt     
no_underflow:
    mov r4, a
    reti

timer1_isr:
    clr tf1
    clr tr1            ; stop the timer
    mov th1, #th1_reload_high 
    mov tl1, #th1_reload_low 
    setb tr1           ; restart the timer

   djnz r2, end1_isr
   mov A, r4
   mov r2, A
   jnb led_pin, call_dec
   acall load_tone_inc
end1_isr:
   reti

call_dec:
   acall load_tone_dec
   jmp end1_isr

load_tone_inc:
   inc r0
   inc r0
   mov a, r0
   mov r1, a
   inc r1

   djnz r3, end_tone_inc
   cpl led_pin
   mov r3, #tone_revert_cnt
   mov r0, #tone_last_h_addr
   mov r1, #tone_last_l_addr
end_tone_inc:
   ret

load_tone_dec:
   dec r0
   dec r0
   mov a, r0
   mov r1, a
   inc r1

   djnz r3, end_tone_dec
   cpl led_pin
   mov r3, #tone_revert_cnt_dec
   mov r0, #tone_first_h_addr
   mov r1, #tone_first_l_addr
end_tone_dec:
   ret

; timer 0 isr (interrupt service routine)
timer0_isr:
    clr tf0
    clr tr0            ; stop the timer
    mov th0, @r0 ; reload high byte
    mov tl0, @r1  ; reload low byte
    cpl buzzer_pin           ; toggle p2.0 (oscillate)
    setb tr0           ; restart the timer
    reti               ; return from interrupt
end