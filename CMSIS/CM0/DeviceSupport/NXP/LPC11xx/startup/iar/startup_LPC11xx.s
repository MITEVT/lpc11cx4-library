/**************************************************
 *
 * Part one of the system initialization code, contains low-level
 * initialization, plain thumb variant.
 *
 * Copyright 2009 IAR Systems. All rights reserved.
 *
 * $Revision: 33183 $
 *
 **************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;
        MODULE  ?cstartup

        ; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)
        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start      
        EXTERN  _FLASHTestPOST
        
        PUBLIC  __vector_table
        PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size
                
        DATA
__vector_table
        DCD     sfe(CSTACK)                 ; Top of Stack
        DCD     Reset_Handler               ; Reset Vector
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     MemManage_Handler           ; MPU Fault Handler
        DCD     BusFault_Handler            ; Bus Fault Handler
        DCD     UsageFault_Handler          ; Usage Fault Handler
__vector_table_0x1c
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     SVC_Handler                 ; SVCall Handler
        DCD     DebugMon_Handler            ; Debug Monitor Handler
        DCD     0                           ; Reserved
        DCD     PendSV_Handler              ; PendSV Handler
        DCD     SysTick_Handler             ; SysTick Handler
        
        ; External Interrupts
        DCD     WAKEUP_IRQHandler         ; 15 wakeup sources for all the
        DCD     WAKEUP_IRQHandler         ; I/O pins starting from PIO0 (0:11)
        DCD     WAKEUP_IRQHandler         ; all 40 are routed to the same ISR                       
        DCD     WAKEUP_IRQHandler                         
        DCD     WAKEUP_IRQHandler                        
        DCD     WAKEUP_IRQHandler
        DCD     WAKEUP_IRQHandler
        DCD     WAKEUP_IRQHandler                       
        DCD     WAKEUP_IRQHandler                         
        DCD     WAKEUP_IRQHandler                        
        DCD     WAKEUP_IRQHandler
        DCD     WAKEUP_IRQHandler
        DCD     WAKEUP_IRQHandler         ; PIO1 (0:11)
        DCD     CAN_IRQHandler            ; CAN                
        DCD     SSP1_IRQHandler           ; SSP1               
        DCD     I2C_IRQHandler            ; I2C
        DCD     TIMER16_0_IRQHandler      ; 16-bit Timer0
        DCD     TIMER16_1_IRQHandler      ; 16-bit Timer1
        DCD     TIMER32_0_IRQHandler      ; 32-bit Timer0
        DCD     TIMER32_1_IRQHandler      ; 32-bit Timer1
        DCD     SSP0_IRQHandler           ; SSP0
        DCD     UART_IRQHandler           ; UART
        DCD     USB_IRQHandler            ; USB IRQ
        DCD     USB_FIQHandler            ; USB FIQ
        DCD     ADC_IRQHandler            ; A/D Converter
        DCD     WDT_IRQHandler            ; Watchdog timer
        DCD     BOD_IRQHandler            ; Brown Out Detect
        DCD     FMC_IRQHandler            ; IP2111 Flash Memory Controller
        DCD     PIOINT3_IRQHandler        ; PIO INT3
        DCD     PIOINT2_IRQHandler        ; PIO INT2
        DCD     PIOINT1_IRQHandler        ; PIO INT1
        DCD     PIOINT0_IRQHandler        ; PIO INT0
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size 	EQU   __Vectors_End - __Vectors          


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        SECTION .text:CODE:REORDER:NOROOT(2)
        PUBLIC  Reset_Handler

WDMOD_ADDR    EQU 0x40004000
WDFEED_ADDR   EQU 0x40004008

Reset_Handler
        LDR     R1, =WDMOD_ADDR   ; load the mod reg address
        LDR     R2, =WDFEED_ADDR  ; load the feed reg address
        MOVS    R5, #0xAA         ; load the pattern
        MVNS    R6, R5            ; load the negated pattern
        MOVS    R4, #0            ; 
        STR     R4, [R1]          ; set mod reg to zero
        STR     R5, [R2]          ; feed the pattern
        STR     R6, [R2]          ; now WD is disabled
 ;       BL      _FLASHTestPOST    ; test the flash
        LDR     R0, = __iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        PUBWEAK HardFault_Handler
        PUBWEAK MemManage_Handler
        PUBWEAK BusFault_Handler
        PUBWEAK UsageFault_Handler
        PUBWEAK SVC_Handler
        PUBWEAK DebugMon_Handler
        PUBWEAK PendSV_Handler
        PUBWEAK SysTick_Handler
        PUBWEAK WAKEUP_IRQHandler
        PUBWEAK CAN_IRQHandler
        PUBWEAK SSP1_IRQHandler
        PUBWEAK I2C_IRQHandler        
        PUBWEAK TIMER16_0_IRQHandler        
        PUBWEAK TIMER16_1_IRQHandler        
        PUBWEAK TIMER32_0_IRQHandler        
        PUBWEAK TIMER32_1_IRQHandler        
        PUBWEAK SSP0_IRQHandler        
        PUBWEAK UART_IRQHandler        
        PUBWEAK USB_IRQHandler        
        PUBWEAK USB_FIQHandler        
        PUBWEAK ADC_IRQHandler        
        PUBWEAK WDT_IRQHandler        
        PUBWEAK BOD_IRQHandler       
        PUBWEAK FMC_IRQHandler
        PUBWEAK PIOINT3_IRQHandler
        PUBWEAK PIOINT2_IRQHandler
        PUBWEAK PIOINT1_IRQHandler
        PUBWEAK PIOINT0_IRQHandler
        
NMI_Handler:
HardFault_Handler:
MemManage_Handler:
BusFault_Handler:
UsageFault_Handler:
SVC_Handler:
DebugMon_Handler:
PendSV_Handler:
SysTick_Handler:
WAKEUP_IRQHandler:
CAN_IRQHandler:
SSP1_IRQHandler:
I2C_IRQHandler:        
TIMER16_0_IRQHandler:        
TIMER16_1_IRQHandler:        
TIMER32_0_IRQHandler:        
TIMER32_1_IRQHandler:        
SSP0_IRQHandler:        
UART_IRQHandler:        
USB_IRQHandler:        
USB_FIQHandler:        
ADC_IRQHandler:        
WDT_IRQHandler:        
BOD_IRQHandler:       
FMC_IRQHandler:
PIOINT3_IRQHandler:
PIOINT2_IRQHandler:
PIOINT1_IRQHandler:
PIOINT0_IRQHandler:
Default_Handler:
        B Default_Handler 
                                                                                                                               
                                                                                                                               
        END
