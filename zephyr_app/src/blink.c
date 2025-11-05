#include "blink.h"
#include <zephyr/drivers/gpio.h>
#include <stdint.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

uint32_t mainled_rate = 150;

static void blink(const struct gpio_dt_spec *led, uint32_t *sleep_ms, uint32_t id)
{
    int ret;

    if (!device_is_ready(led->port))
    {
        return;
    }

    ret = gpio_pin_configure_dt(led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        return;
    }

    while (1)
    {
        // if the rate is zero we set the LED OFF
        if (*sleep_ms == 0)
        {
            gpio_pin_set_dt(led, 0);
            // and we sleep some time to let the task run
            k_msleep(200);
        }
        else
        {
            if (*sleep_ms == 1)
            {
                // Led always on
                gpio_pin_set_dt(led, 1);
                // and we sleep some time to let the task run
                k_msleep(200);
            }
            else
            {
                // Otherwise we just blink the led at the defined rate
                gpio_pin_toggle_dt(led);
                // printk("Blink %d\n", id);
                k_msleep(*sleep_ms);
            }
        }
    }
}

void blink_handler(void)
{

    blink(&led0, &mainled_rate, 0);
}

