/**
 * @file vesync_task_def.h
 * @brief 任务名称、优先级、堆栈大小的统一宏定义
 * @author WatWu
 * @date 2018-11-07
 */

#ifndef VESYNC_TASK_DEF_H
#define VESYNC_TASK_DEF_H

typedef enum
{
	TASK_PRIORITY_IDLE = 0,                                 /* lowest, special for idle task */

	/* User task priority begin, please define your task priority at this interval */
	TASK_PRIORITY_LOW,                                      /* low */
	TASK_PRIORITY_BELOW_NORMAL,                             /* below normal */
	TASK_PRIORITY_NORMAL,                                   /* normal */
	TASK_PRIORITY_ABOVE_NORMAL,                             /* above normal */
	TASK_PRIORITY_HIGH,                                     /* high */
	TASK_PRIORITY_SOFT_REALTIME,                            /* soft real time */
	TASK_PRIORITY_HARD_REALTIME,                            /* hard real time */
	/* User task priority end */

	/*Be careful, the max-priority number can not be bigger than configMAX_PRIORITIES - 1, or kernel will crash!!! */
	TASK_PRIORITY_TIMER = configMAX_PRIORITIES - 1,         /* highest, special for timer task to keep time accuracy */
} task_priority_type_t;

/*
The following is an example to define the XXXX task.
#define XXXX_TASK_NAME "XXXX"
#define XXXX_TASK_STACKSIZE 512
#define XXXX_TASK_PRIO TASK_PRIORITY_LOW
#define XXXX_QUEUE_LENGTH  16
*/

/* part_1: SDK tasks configure infomation, please don't modify */

/* part_2: Application and customer tasks configure information */

#define VESYNC_TASK_NAME				"vesync"
#define VESYNC_TASK_STACKSIZE           (1024*2)
#define VESYNC_TASK_PRIO                TASK_PRIORITY_NORMAL

#define EVENT_TASK_NAME    		        "event_center"
#define EVENT_TASK_STACSIZE		        (1024*4)
#define EVENT_TASK_PRIO    		        TASK_PRIORITY_HARD_REALTIME

#define DEVELOPER_TASK_NAME    	        "developer"
#define DEVELOPER_TASK_STACSIZE	        (1024*8)
#define DEVELOPER_TASK_PRIO    	        TASK_PRIORITY_SOFT_REALTIME

#define TCP_SERVER_TASK_NAME    	    "tcp_server"
#define TCP_SERVER_TASK_STACSIZE	    (1024*2)
#define TCP_SERVER_TASK_PRIO    	    TASK_PRIORITY_NORMAL

#endif
