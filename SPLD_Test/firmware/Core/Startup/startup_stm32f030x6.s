/**
  ******************************************************************************
  * @file      startup_stm32f030x6.s
  * @author    MCD Application Team
  * @brief     STM32F030x4/x6 devices vector table for GCC toolchain.
  *            Same vector table as IAR/EWARM, section name .isr_vector for linker.
  ******************************************************************************
  */
  .syntax unified
  .cpu cortex-m0
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/* start address for the initialization values of the .data section.
   defined in linker script */
.word _sidata
/* start address for the .data section. defined in linker script */
.word _sdata
/* end address for the .data section. defined in linker script */
.word _edata
/* start address for the .bss section. defined in linker script */
.word _sbss
/* end address for the .bss section. defined in linker script */
.word _ebss

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:
  ldr   r0, =_estack
  mov   sp, r0
  ldr   r0, =SystemInit
  blx   r0
  ldr   r2, =_sdata
  ldr   r3, =_edata
  ldr   r4, =_sidata
  b     .L_loop_data
.L_copy_data:
  ldr   r1, [r4]
  adds  r4, r4, #4
  str   r1, [r2]
  adds  r2, r2, #4
.L_loop_data:
  cmp   r2, r3
  bcc   .L_copy_data
  ldr   r2, =_sbss
  ldr   r3, =_ebss
  movs  r1, #0
  b     .L_loop_bss
.L_fill_bss:
  str   r1, [r2]
  adds  r2, r2, #4
.L_loop_bss:
  cmp   r2, r3
  bcc   .L_fill_bss
  bl    main
  bx    lr
.size Reset_Handler, .-Reset_Handler

  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word 0
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  .word WWDG_IRQHandler
  .word 0
  .word RTC_IRQHandler
  .word FLASH_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_1_IRQHandler
  .word EXTI2_3_IRQHandler
  .word EXTI4_15_IRQHandler
  .word 0
  .word DMA1_Channel1_IRQHandler
  .word DMA1_Channel2_3_IRQHandler
  .word DMA1_Channel4_5_IRQHandler
  .word ADC1_IRQHandler
  .word TIM1_BRK_UP_TRG_COM_IRQHandler
  .word TIM1_CC_IRQHandler
  .word 0
  .word TIM3_IRQHandler
  .word 0
  .word 0
  .word TIM14_IRQHandler
  .word 0
  .word TIM16_IRQHandler
  .word TIM17_IRQHandler
  .word I2C1_IRQHandler
  .word 0
  .word SPI1_IRQHandler
  .word 0
  .word USART1_IRQHandler

  .thumb_func
  .weak Default_Handler
  .type Default_Handler, %function
Default_Handler:
  b .
  .size Default_Handler, .-Default_Handler

  .macro def_irq_handler handler_name
  .weak \handler_name
  .set  \handler_name, Default_Handler
  .endm

  def_irq_handler NMI_Handler
  def_irq_handler HardFault_Handler
  def_irq_handler SVC_Handler
  def_irq_handler PendSV_Handler
  def_irq_handler SysTick_Handler
  def_irq_handler WWDG_IRQHandler
  def_irq_handler RTC_IRQHandler
  def_irq_handler FLASH_IRQHandler
  def_irq_handler RCC_IRQHandler
  def_irq_handler EXTI0_1_IRQHandler
  def_irq_handler EXTI2_3_IRQHandler
  def_irq_handler EXTI4_15_IRQHandler
  def_irq_handler DMA1_Channel1_IRQHandler
  def_irq_handler DMA1_Channel2_3_IRQHandler
  def_irq_handler DMA1_Channel4_5_IRQHandler
  def_irq_handler ADC1_IRQHandler
  def_irq_handler TIM1_BRK_UP_TRG_COM_IRQHandler
  def_irq_handler TIM1_CC_IRQHandler
  def_irq_handler TIM3_IRQHandler
  def_irq_handler TIM14_IRQHandler
  def_irq_handler TIM16_IRQHandler
  def_irq_handler TIM17_IRQHandler
  def_irq_handler I2C1_IRQHandler
  def_irq_handler SPI1_IRQHandler
  def_irq_handler USART1_IRQHandler

  .end
