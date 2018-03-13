/*
 * FreeRTOSConfig.h
 *
 *  Created on: 9 mar. 2018
 *      Author: Daniel
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

//#include "FreeRTOS.h"

#define configUSE_PREEMPTION                    1   //Set to 1 to use the preemptive RTOS scheduler, or 0 to use the cooperative RTOS scheduler.
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0   //Some FreeRTOS ports have two methods of selecting the next task to execute - a generic method, and a method that is specific to that port.
#define configUSE_TICKLESS_IDLE                 0   //Set configUSE_TICKLESS_IDLE to 1 to use the low power tickless mode, or 0 to keep the tick interrupt running at all times. Low power tickless implementations are not provided for all FreeRTOS ports.
#define configCPU_CLOCK_HZ                      ( (unsigned long) 40000000 )    //Set to the number of Hz the CPU is using for the application
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )     //The frequency of the RTOS tick interrupt.
#define configMAX_PRIORITIES                    5       //The number of priorities available to the application tasks. Any number of tasks can share the same priority. Co-routines are prioritised separately - see configMAX_CO_ROUTINE_PRIORITIES.
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )     //The size of the stack used by the idle task. Generally this should not be reduced from the value set in the FreeRTOSConfig.h file provided with the demo application for the port you are using.
#define configMAX_TASK_NAME_LEN                 16      //The maximum permissible length of the descriptive name given to a task when the task is created. The length is specified in the number of characters including the NULL termination byte.
#define configUSE_16_BIT_TICKS                  0       //Time is measured in 'ticks' - which is the number of times the tick interrupt has executed since the RTOS kernel was started. The tick count is held in a variable of type TickType_t.
#define configIDLE_SHOULD_YIELD                 1       //This parameter controls the behaviour of tasks at the idle priority.
#define configUSE_TASK_NOTIFICATIONS            1       //Setting configUSE_TASK_NOTIFICATIONS to 1 (or leaving configUSE_TASK_NOTIFICATIONS undefined) will include direct to task notification functionality and its associated API in the build.
#define configUSE_MUTEXES                       0       //Set to 1 to include mutex functionality in the build, or 0 to omit mutex functionality from the build. Readers should familiarise themselves with the differences between mutexes and binary semaphores in relation to the FreeRTOS functionality.
#define configUSE_RECURSIVE_MUTEXES             0       //Set to 1 to include recursive mutex functionality in the build, or 0 to omit recursive mutex functionality from the build.
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_ALTERNATIVE_API               0 /* Deprecated! */
#define configQUEUE_REGISTRY_SIZE               10
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  0
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 20 * 1024 ) )
#define configAPPLICATION_ALLOCATED_HEAP        0

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

/* Interrupt nesting behaviour configuration. */
#define configKERNEL_INTERRUPT_PRIORITY         ( 7 << 5 )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 4 << 5 )
#define configMAX_API_CALL_INTERRUPT_PRIORITY   ( 4 << 5 )


/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1

/* A header file that defines trace macro can be included here. */
/* Define configASSERT() to disable interrupts and sit in a loop. */
//#define configASSERT( (x)  )     if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); while(1); }

#endif /* FREERTOS_CONFIG_H */
