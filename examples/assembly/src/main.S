/*****************************************************************************/
/* startup_LPC11xx.s: Startup file for ARM CM0 device series                 */
/*****************************************************************************/

/* Stack */

    .section .stack
    .align  3
#ifdef __STACK_SIZE
    .equ    Stack_Size, __STACK_SIZE
#else
    .equ    Stack_Size, 0x200
#endif
    .globl  __StackTop
    .globl  __StackLimit
__StackLimit:
    .space  Stack_Size
    .size   __StackLimit, . - __StackLimit
__StackTop:
    .size   __StackTop, . - __StackTop

/* Vector Table */

    .section .isr_vector
    .align 2
    .globl  __isr_vector
__isr_vector:
    .long   __StackTop            /* Top of Stack */
    .long   Reset_Handler         /* Reset Handler */
    .long   NMI_Handler           /* NMI Handler */
    .long   HardFault_Handler     /* Hard Fault Handler */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   SVC_Handler           /* SVCall Handler */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   PendSV_Handler        /* PendSV Handler */
    .long   SysTick_Handler       /* SysTick Handler */


    /* External Interrupts */
    .long   WAKEUP_IRQHandler           /* 16+ 0: Wakeup PIO0.0          */
    .long   WAKEUP_IRQHandler           /* 16+ 1: Wakeup PIO0.1          */
    .long   WAKEUP_IRQHandler           /* 16+ 2: Wakeup PIO0.2          */
    .long   WAKEUP_IRQHandler           /* 16+ 3: Wakeup PIO0.3          */
    .long   WAKEUP_IRQHandler           /* 16+ 4: Wakeup PIO0.4          */
    .long   WAKEUP_IRQHandler           /* 16+ 5: Wakeup PIO0.5          */
    .long   WAKEUP_IRQHandler           /* 16+ 6: Wakeup PIO0.6          */
    .long   WAKEUP_IRQHandler           /* 16+ 7: Wakeup PIO0.7          */
    .long   WAKEUP_IRQHandler           /* 16+ 8: Wakeup PIO0.8          */
    .long   WAKEUP_IRQHandler           /* 16+ 9: Wakeup PIO0.9          */
    .long   WAKEUP_IRQHandler           /* 16+10: Wakeup PIO0.10         */
    .long   WAKEUP_IRQHandler           /* 16+11: Wakeup PIO0.11         */
    .long   WAKEUP_IRQHandler           /* 16+12: Wakeup PIO1.0          */
    .long   CAN_IRQHandler             	/* 16+13: CAN Handler            */
    .long   SSP1_IRQHandler        		/* 16+14: SSP1 Handler			 */
    .long   I2C_IRQHandler		        /* 16+15: I2C Handler			 */
    .long   TIMER16_0_IRQHandler        /* 16+16: 16-bit Counter-Timer 0 */
    .long   TIMER16_1_IRQHandler        /* 16+17: 16-bit Counter-Timer 1 */
    .long   TIMER32_0_IRQHandler        /* 16+18: 32-bit Counter-Timer 0 */
    .long   TIMER32_1_IRQHandler        /* 16+19: 32-bit Counter-Timer 1 */
    .long   SSP0_IRQHandler             /* 16+20: SSP0 Handler           */
    .long   UART_IRQHandler             /* 16+21: UART Handler           */
    .long   0              				/* 16+22: Reserved          	 */
    .long   0              				/* 16+23: Reserved         		 */
    .long   ADC_IRQHandler              /* 16+24: ADC Handler	         */
    .long   WDT_IRQHandler              /* 16+25: Watchdog Handler       */
    .long   BOD_IRQHandler          	/* 16+26: Brown-out detect       */
    .long   0          					/* 16+27: Reserved               */
    .long   PIOINT3_IRQHandler          /* 16+28: PIO INT3               */
    .long   PIOINT2_IRQHandler          /* 16+29: PIO INT2               */
    .long   PIOINT1_IRQHandler          /* 16+30: PIO INT2               */
    .long   PIOINT0_IRQHandler          /* 16+31: PIO INT2               */
    

    .size   __isr_vector, . - __isr_vector


    .thumb

    .thumb_func
    .align  2
    .globl  Main
    .type   Main, %function

Main:
    .fnstart
/* -- Main Code Goes Here -- */

    mov r7, #0      /* Zero Register */

/* Enable GPIO */
    /* SYSAHBCLKCTRL  0x4004 8080 */
    ldr r0, =0x40048080

    movs r1, #0x40       /* GPIO */
    ldr r2, [r0, #0]          /* Get Value */
    orr r2, r1         /* SYSAHBCLKCTRL | GPIO_INIT_MASK */
    str r2, [r0, #0]         /* Store */

/* Write Dir Bit Pin 2_8 */
    /* GPIO2DIR: 0x5002 8000 */
    ldr r0, =0x50028000

    movs r1, #0x80 /* (1 << 8) */
    lsl r1, #1
    str r1, [r0, #0]


/* Set Pin High */
    /* GPIO2DATA: 0x5002 0000 - 0x5002 3FFC = 0x5002 0400 = bit 8 left shifted 2*/
    ldr r0, =0x50023ffc
    str r1, [r0, #0]

.hang:
    b .hang
    


    /* -- Main Code Ends Here -- */
    .pool
    .cantunwind
    .fnend
    .size   Main, . - Main

    .thumb_func
    .align  1
    .globl  Reset_Handler
    .type   Reset_Handler, %function




/* Reset Handler */

Reset_Handler:
    .fnstart

#ifdef __STARTUP_CLEAR_BSS
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    movs r0, #0             /* clear r0 */

    sub r2, r1              /* r2 = length of bss */
    ble .L_loop3_done       /* if length <= 0, done (no bss) */

.L_loop3:
    sub r2, #4              /* r2 = r2 - 4 */
    /* r0 -> (r1 + r2)
        put value of r0 (zero) into address made by summing r1 and r2 (bss start pointer + decreasing offset) */
    str r0, [r1, r2] 
    bgt .L_loop3
.L_loop3_done:
#endif /* __STARTUP_CLEAR_BSS */


#ifndef __NO_SYSTEM_INIT
    bl  SystemInit
#endif


    bl Main    /* Why make a function? Makes Reset Handler too big? */


    .pool
    .cantunwind
    .fnend
    .size   Reset_Handler, . - Reset_Handler

    .section .text

/* Exception Handlers */

    .weak   NMI_Handler
    .type   NMI_Handler, %function
NMI_Handler:
    B       .
    .size   NMI_Handler, . - NMI_Handler

    .weak   HardFault_Handler
    .type   HardFault_Handler, %function
HardFault_Handler:
    B       .
    .size   HardFault_Handler, . - HardFault_Handler

    .weak   SVC_Handler
    .type   SVC_Handler, %function
SVC_Handler:
    B       .
    .size   SVC_Handler, . - SVC_Handler

    .weak   PendSV_Handler
    .type   PendSV_Handler, %function
PendSV_Handler:
    B       .
    .size   PendSV_Handler, . - PendSV_Handler

    .weak   SysTick_Handler
    .type   SysTick_Handler, %function
SysTick_Handler:
    B       .
    .size   SysTick_Handler, . - SysTick_Handler


/* IRQ Handlers */

    .globl  Default_Handler
    .type   Default_Handler, %function
Default_Handler:
    B       .
    .size   Default_Handler, . - Default_Handler

    .macro  IRQ handler
    .weak   \handler
    .set    \handler, Default_Handler
    .endm

    IRQ     WAKEUP_IRQHandler
    IRQ     CAN_IRQHandler
    IRQ     SSP1_IRQHandler
    IRQ		I2C_IRQHandler
    IRQ     TIMER16_0_IRQHandler
    IRQ     TIMER16_1_IRQHandler
    IRQ     TIMER32_0_IRQHandler
    IRQ     TIMER32_1_IRQHandler
    IRQ     SSP0_IRQHandler
    IRQ     UART_IRQHandler
    IRQ     ADC_IRQHandler
    IRQ     WDT_IRQHandler
    IRQ     BOD_IRQHandler
    IRQ     PIOINT3_IRQHandler
    IRQ     PIOINT2_IRQHandler
    IRQ     PIOINT1_IRQHandler
    IRQ     PIOINT0_IRQHandler


    .end