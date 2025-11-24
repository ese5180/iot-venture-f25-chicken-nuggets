#include "blink.h"
#include "color.h"
#include "lorawan.h"
#include <zephyr/kernel.h>

/* size of stack area used by each thread */
#define BLINK_STACKSIZE 1024
#define LORAWAN_STACKSIZE 8192
#define COLOR_STACKSIZE 2048

extern void color_handler(void);

/* scheduling priority used by each thread */
/* Lower priority is higher */
#define BLINK_PRIORITY 7
#define LORAWAN_PRIORITY 5
#define COLOR_PRIORITY 6

// LED Blinking Handler Task
K_THREAD_DEFINE(blink_handler_id, BLINK_STACKSIZE, blink_handler, NULL, NULL,
                NULL, BLINK_PRIORITY, 0, 0); // Main Green LED

// LoRaWAN Handler Task
K_THREAD_DEFINE(lorawan_handler_id, LORAWAN_STACKSIZE, lorawan_handler, NULL,
                NULL, NULL, LORAWAN_PRIORITY, 0, 0);

// Color Sensor Task
K_THREAD_DEFINE(color_thread_id, 2048, color_handler, NULL, NULL, NULL, 6, 0,
                0);

// #include <zephyr/kernel.h>
// #include "color.h"

// // Thread stack + control block
// #define COLOR_STACK_SIZE 2048
// #define COLOR_PRIORITY 5

// K_THREAD_STACK_DEFINE(color_stack, COLOR_STACK_SIZE);
// static struct k_thread color_thread;

// void main(void)
// {
//     printk("Main started. Launching color sensor thread...\n");

//     k_thread_create(
//         &color_thread,
//         color_stack,
//         K_THREAD_STACK_SIZEOF(color_stack),
//         (k_thread_entry_t)color_handler,
//         NULL, NULL, NULL,
//         COLOR_PRIORITY,
//         0,
//         K_NO_WAIT
//     );

//     printk("Color thread launched.\n");
// }