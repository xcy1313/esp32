/* ULP assembly files are passed through C preprocessor first, so include directives
   and C macros may be used in these files 
 */
#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"

/*-------------------------------------------------------------------------------------*/
/* 宏定义段 */
	.set adc_channel, 4							/* 定义ADC通道，ADC1 channel 4, GPIO32 */
	.set adc_oversampling_factor_log, 5			/* 启动一次ADC连续采样的样本数，左移5位，即32 */
	.set adc_oversampling_factor, (1 << adc_oversampling_factor_log)
	.set adc_buff_size, 20						/* ADC采样缓存buffer的大小 */

/*-------------------------------------------------------------------------------------*/
/* 数据段 */
	/* Define variables, which go into .bss section (zero-initialized data) */
	.bss

	.global adc_min								/* ADC缓存buffer中的最小值 */
adc_min:.long 0

	.global adc_max								/* ADC缓存buffer中的最大值 */
adc_max:.long 0

	.global threshold							/* adc连续采样时的最大变化阈值 */
threshold:.long 0
	
	.global all_sample_counter					/* 总的ADC采样累计值 */
all_sample_counter:.long 0
	
	.global wakeup_counter						/* 唤醒统计次数 */
wakeup_counter:.long 0
	
	.global last_result							/* 最近一次的测量值 */
last_result:.long 0
	
	.global adc_pos								/* ADC缓存buffer当前位置 */
adc_pos:.long 0
	
	.global adc_buff							/* adc采样缓存buffer，保存最近一定数量的采样平均值值 */
adc_buff:
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0

	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0

/*-------------------------------------------------------------------------------------*/
/* 代码段 */
	.text
	.global entry
entry:
	move r3, all_sample_counter					/* 总的ADC样本累计值加1 */
	ld r2, r3, 0
	add r2, r2, 1
	st r2, r3, 0

	move r1, 0									/* r1 寄存器作为ADC采样累加器，清零 */
	stage_rst									/* 复位阶段计数寄存器 */

/* ADC测量 */
measure:
	adc r0, 0, adc_channel + 1					/* 读取ADC值到r0并累加到r1中 */
	add r1, r1, r0
	stage_inc 1									/* 阶段计数器递增1 */
	jumps measure, adc_oversampling_factor, lt	/* 判断阶段计数器是否超过了设定的阈值，没超则跳转继续读取ADC */
	rsh r1, r1, adc_oversampling_factor_log		/* 累加了adc_oversampling_factor个样本，右移adc_oversampling_factor_log求平均值 */
	move r3, last_result
	st r1, r3, 0								/* 完成了一次测量，采样了adc_oversampling_factor个样本，结果存放到了r1及last_result中 */

/* ADC测量值保存到缓存buffer */
save_to_buff:
	move r3, adc_pos
	ld r2, r3, 0								/* 读取当前数组下标到r2 */
	move r0, adc_buff							/* r0保存buffer初始地址 */
	add r0, r0, r2								/* 偏移至数组下标位置 */
	st r1, r0, 0								/* 保存上次测量值到buffer的对应数组下标位置中 */
	ld r0, r3, 0								/* 重新获取buffer的数组下标值到r0 */
	add r0, r0, 1								/* 下标值递增1 */
	jumpr reset_adc_pos, adc_buff_size, ge		/* 判断r0中的值即数组下标值是否累加到数组大小，是则复位，否则继续 */
	st r0, r3, 0								/* 保存递增的数组下标 */
	jump continue_1
reset_adc_pos:
	move r3, adc_pos							/* 复位数组下标值 */
	move r2, 0
	st r2, r3, 0

continue_1:
	move r3, all_sample_counter
	ld r0, r3, 0
	jumpr exit, adc_buff_size, lt				/* 判断总的采样次数是否大于缓存buffer的大小，小于的话直接跳过阈值判断 */

	stage_rst
	move r3, adc_buff
	ld r0, r3, 0								/* r0保存遍历数组过程中的最小值，首先保存第一位 */
/* 获取数组中的最小值 */
get_buffer_min:
	ld r2, r3, 0								/* 读取buffer下一位到r2 */
	sub r1, r2, r0								/* r2-r0，溢出了代表r2更小 */
	jump sub_overflow, ov
	jump continue_2								/* 没溢出，r0则还是最小的，保持不变 */
sub_overflow:
	move r0, r2									/* r2更小，赋值给到r0 */
continue_2:
	add r3, r3, 1								/* buffer偏移1位 */
	stage_inc 1									/* 阶段计数器递增1，递增到buffer大小后结束 */
	jumps get_buffer_min, adc_buff_size, lt
	move r3, adc_min							/*获取到的数组中最小值保存到adc_min*/
	st r0, r3, 0

	stage_rst
	move r3, adc_buff
	ld r0, r3, 0								/* r0保存遍历数组过程中的最大值，首先保存第一位 */
/* 获取数组中的最大值 */
get_buffer_max:
	ld r2, r3, 0								/* 读取buffer下一位到r2 */
	sub r1, r0, r2								/* r0-r2，溢出了代表r2更大 */
	jump sub_overflow_2, ov
	jump continue_3								/* 没溢出，r0则还是最大的，保持不变 */
sub_overflow_2:
	move r0, r2									/* r2更大，赋值给到r0 */
continue_3:
	add r3, r3, 1								/* buffer偏移1位 */
	stage_inc 1									/* 阶段计数器递增1，递增到buffer大小后结束 */
	jumps get_buffer_max, adc_buff_size, lt
	move r3, adc_max							/*获取到的数组中最大值保存到adc_max*/
	st r0, r3, 0

/* 判断最大最小值的差值是否超出预设阈值 */
judge_threshold:
	move r3, threshold
	ld r2, r3, 0								/* 取阈值到r2 */
	move r3, adc_max
	ld r1, r3, 0								/* 取最大值到r1 */
	move r3, adc_min
	ld r0, r3, 0								/* 取最小值到r0 */
	sub r1, r1, r0								/* 求最大值和最小值的差值到r1 */
	sub r1, r2, r1								/* 阈值减差值，溢出则代表变化超出了阈值 */
	jump threshold_overflow, ov
	jump continue_4
threshold_overflow:
	jump wake_up								/* 人体红外检测变化超出阈值，唤醒主CPU */

continue_4:
	/* value within range, end the program */
	.global exit
exit:
	halt

	.global wake_up
wake_up:
	/* Check if the system can be woken up */
	READ_RTC_FIELD(RTC_CNTL_LOW_POWER_ST_REG, RTC_CNTL_RDY_FOR_WAKEUP)
	and r0, r0, 1
	jump exit, eq

	/* Wake up the SoC, end program */
	wake
	WRITE_RTC_FIELD(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN, 0)
	move r3, wakeup_counter
	ld r2, r3, 0
	add r2, r2, 1
	st r2, r3, 0
	halt
