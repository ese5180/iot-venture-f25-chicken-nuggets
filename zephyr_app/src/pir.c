#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "pir.h"

LOG_MODULE_REGISTER(pir, LOG_LEVEL_INF);

/* Get PIR GPIO from DTS:
	 /{
		 zephyr,user { pir_gpios = <&gpio0 28 (GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH)>; };
	 };
*/
#define USER_NODE DT_PATH(zephyr_user)
BUILD_ASSERT(DT_NODE_HAS_PROP(USER_NODE, pir_gpios), "pir_gpios missing in DTS");
static const struct gpio_dt_spec pir = GPIO_DT_SPEC_GET(USER_NODE, pir_gpios);

static struct gpio_callback pir_cb;
static K_SEM_DEFINE(pir_sem, 0, 1);

static void pir_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);
	k_sem_give(&pir_sem);
}

/* Thread: waits for motion and logs it. */
void pir_handler(void *a, void *b, void *c)
{
	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	if (!device_is_ready(pir.port))
	{
		LOG_ERR("PIR GPIO controller not ready");
		return;
	}

	int ret = gpio_pin_configure_dt(&pir, GPIO_INPUT);
	if (ret)
	{
		LOG_ERR("gpio_pin_configure_dt: %d", ret);
		return;
	}

	/* Rising edge => motion = HIGH. If your module is active-low, swap to
		 GPIO_INT_EDGE_TO_INACTIVE. */
	ret = gpio_pin_interrupt_configure_dt(&pir, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret)
	{
		LOG_ERR("irq config: %d", ret);
		return;
	}

	gpio_init_callback(&pir_cb, pir_isr, BIT(pir.pin));
	gpio_add_callback(pir.port, &pir_cb);

	LOG_INF("PIR ready. Warm-up ~30s…");

	while (1)
	{
		k_sem_take(&pir_sem, K_FOREVER);
		LOG_INF("Motion detected!");
		/* TODO: add your action here (e.g., enqueue LoRa uplink) */
		printk("Motion detected!\n");
	}
}
