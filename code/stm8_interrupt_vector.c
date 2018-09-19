#include "include.h"


typedef void @far (*interrupt_handler_t)(void);

struct interrupt_vector {
	unsigned char interrupt_instruction;
	interrupt_handler_t interrupt_handler;
};

#if 0
@far @interrupt void NonHandledInterrupt (void)
{
	/* in order to detect unexpected events during development, 
	   it is recommended to set a breakpoint on the following instruction
	*/
	return;
}
#endif

/**
  * @brief Dummy interrupt routine
  * @par Parameters:
  * None
  * @retval void None
  * @par Required preconditions:
  * None
  * @par Called functions:
  * None
*/

@near @interrupt void NonHandledInterrupt(void)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  return;
}

extern void _stext();     /* startup routine */

struct interrupt_vector const _vectab[] = {
	{0x82, (interrupt_handler_t)_stext}, /* reset */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* trap  */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq0  */
	{0x82, (interrupt_handler_t)AWU_IRQHandler}, /* irq1  */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq2  */
	{0x82, (interrupt_handler_t)EXTI_PORTA_IRQHandler}, /* irq3  */
	{0x82, (interrupt_handler_t)EXTI_PORTB_IRQHandler}, /* irq4  */
	{0x82, (interrupt_handler_t)EXTI_PORTC_IRQHandler}, /* irq5  */
	{0x82, (interrupt_handler_t)EXTI_PORTD_IRQHandler}, /* irq6  */
	{0x82, (interrupt_handler_t)EXTI_PORTE_IRQHandler}, /* irq7  */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq8  */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq9  */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq10 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq11 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq12 */
	{0x82, (interrupt_handler_t)TIM2_UPD_OVF_BRK_IRQHandler}, /* irq13 */
	{0x82, (interrupt_handler_t)TIM2_CAP_COM_IRQHandler}, /* irq14 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq15 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq16 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq17 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq18 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq19 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq20 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq21 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq22 */
	{0x82, (interrupt_handler_t)TIM4_UPD_OVF_IRQHandler}, /* irq23 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq24 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq25 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq26 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq27 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq28 */
	{0x82, (interrupt_handler_t)NonHandledInterrupt}, /* irq29 */
};

