/* ***************************************************************************************************************

	crt.s						STARTUP  ASSEMBLY  CODE 
								-----------------------


	Module includes the interrupt vectors and start-up code.

  *************************************************************************************************************** */

.section .startup, "ax"

.global	Reset_Handler
.global _startup
.func   _startup

_startup:

# Exception Vectors

        .thumb
        .syntax unified

		.extern void NmiSR      
		.extern void FaultISR   
		.extern void FaultMPU   
		.extern void FaultBUS   
		.extern void FaultUSE
		.extern void IntDefaultHandler      

		.extern void uart0ISR
_vectors:
        .word     _stack_end                  // 0 : Top of Stack
        .word     Reset_Handler               // 1 : Reset Handler
        .word     NmiSR                       // 2 : NMI Handler
        .word     FaultISR                    // 3 : Hard Fault Handler
        .word     FaultMPU                    // 4 : The MPU fault handler
        .word     FaultBUS                    // 5 : The bus fault handler
        .word     FaultUSE                    // 6 : The usage fault handler
        .word     0                           // 7 : Reserved
        .word     0                           // 8 : Reserved
        .word     0                           // 9 : Reserved
        .word     0                           // 10: Reserved
        .word     IntDefaultHandler           // 11: SVCall handler
        .word     IntDefaultHandler           // 12: Debug monitor handler
        .word     0                           // 13: Reserved
        .word     IntDefaultHandler           // 14: The PendSV handler
        .word     IntDefaultHandler           // 15: The SysTick handler
        .word     IntDefaultHandler           // 16: Watchdog Timer
        .word     IntDefaultHandler           // 17: Timer0
        .word     IntDefaultHandler           // 18: Timer1
        .word     IntDefaultHandler           // 19: Timer2
        .word     IntDefaultHandler           // 20: Timer3
        .word     uart0ISR                    // 21: UART0
        .word     IntDefaultHandler           // 22: UART1
        .word     IntDefaultHandler           // 23: UART2
        .word     IntDefaultHandler           // 24: UART3
        .word     IntDefaultHandler           // 25: PWM1
        .word     IntDefaultHandler           // 26: I2C0
        .word     IntDefaultHandler           // 27: I2C1
        .word     IntDefaultHandler           // 28: I2C2
        .word     IntDefaultHandler           // 29: SPI
        .word     IntDefaultHandler           // 30: SSP0
        .word     IntDefaultHandler           // 31: SSP1
        .word     IntDefaultHandler           // 32: PLL0 Lock (Main PLL)
        .word     IntDefaultHandler           // 33: Real Time Clock
        .word     IntDefaultHandler           // 34: External Interrupt 0
        .word     IntDefaultHandler           // 35: External Interrupt 1
        .word     IntDefaultHandler           // 36: External Interrupt 2
        .word     IntDefaultHandler           // 37: External Interrupt 3
        .word     IntDefaultHandler           // 38: A/D Converter
        .word     IntDefaultHandler           // 39: Brown-Out Detect
        .word     IntDefaultHandler           // 40: USB
        .word     IntDefaultHandler           // 41: CAN
        .word     IntDefaultHandler           // 42: General Purpose DMA
        .word     IntDefaultHandler           // 43: I2S
        .word     IntDefaultHandler           // 44: Ethernet
        .word     IntDefaultHandler           // 45: Repetitive Interrupt Timer
        .word     IntDefaultHandler           // 46: Motor Control PWM
        .word     IntDefaultHandler           // 47: Quadrature Encoder Interface
        .word     IntDefaultHandler           // 48: PLL1 Lock (USB PLL)
.thumb_func
Reset_Handler:

				/* copy .data section (Copy from ROM to RAM) */
                ldr     R1, =_etext
                ldr     R2, =_data
                ldr     R3, =_edata
1:        		cmp     R2, R3
                ldr   R0, [R1], #4
                str   R0, [R2], #4
                blo     1b

				/* Clear .bss section (Zero init)  */
                mov     R0, #0
                ldr     R1, =_bss_start
                ldr     R2, =_bss_end
2:				cmp     R1, R2
                str   R0, [R1], #4
                blo     2b

				/* Enter the C code  */
                b       main


.endfunc
.end
