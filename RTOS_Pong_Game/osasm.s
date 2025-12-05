;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 2 starter file
; February 10, 2016
;


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler


SysTick_Handler               
    CPSID   I                  
    PUSH    {R4-R11}           
    LDR     R0, =RunPt        
    LDR     R1, [R0]           
    STR     SP, [R1]          
    PUSH    {R0,LR}            
    BL      Scheduler         
    POP     {R0,LR}            
    LDR     R0, =RunPt           
    ;STR     R1, [R0]  Old Bennett
	LDR     R1, [R0]
    LDR     SP, [R1]           
    POP     {R4-R11}           
    CPSIE   I                  
    BX      LR                 

StartOS
	LDR     R0, =RunPt        
    LDR     R1, [R0]         
    LDR     SP, [R1]          
    POP     {R4-R11}          
    POP     {R0-R3}          
    POP     {R12}
    ADD     SP,SP,#4           
    POP     {LR}               
    ADD     SP,SP,#4           
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread

    ALIGN
    END
