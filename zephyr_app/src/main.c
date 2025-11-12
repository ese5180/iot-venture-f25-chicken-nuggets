#include <zephyr/kernel.h>
#include "lorawan.h"
#include "blink.h"
#include "wifi_ota.h"

/* size of stack area used by each thread */
#define BLINK_STACKSIZE 1024
#define LORAWAN_STACKSIZE 8192

/* scheduling priority used by each thread */
/* Lower priority is higher */
#define BLINK_PRIORITY 7
#define LORAWAN_PRIORITY 5

// LED Blinking Handler Task
K_THREAD_DEFINE(blink_handler_id, BLINK_STACKSIZE, blink_handler, NULL, NULL, NULL, BLINK_PRIORITY, 0, 0); // Main Green LED

// LoRaWAN Handler Task
K_THREAD_DEFINE(lorawan_handler_id, LORAWAN_STACKSIZE, lorawan_handler, NULL, NULL, NULL, LORAWAN_PRIORITY, 0, 0);

// Wifi OTA Task
K_THREAD_DEFINE(wifi_ota_handler_id, LORAWAN_STACKSIZE, wifi_ota_handler, NULL, NULL, NULL, LORAWAN_PRIORITY, 0, 0);
