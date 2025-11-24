#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include "color.h"

void color_handler(void)
{
    const struct device *dev = DEVICE_DT_GET_ONE(avago_apds9960);

    if (!dev) {
        printk("APDS9960 not found in Devicetree!\n");
        return;
    }

    if (!device_is_ready(dev)) {
        printk("APDS9960 device not ready!\n");
        return;
    }

    printk("APDS9960 Color Thread Started\n");

    while (1) {
        struct sensor_value r, g, b;
        struct sensor_value ambient;
        struct sensor_value prox;

        if (sensor_sample_fetch(dev) < 0) {
            printk("APDS9960 fetch error\n");
            k_msleep(100);
            continue;
        }

        // RGB channels
        sensor_channel_get(dev, SENSOR_CHAN_RED,   &r);
        sensor_channel_get(dev, SENSOR_CHAN_GREEN, &g);
        sensor_channel_get(dev, SENSOR_CHAN_BLUE,  &b);

        // Ambient (clear light level)
        sensor_channel_get(dev, SENSOR_CHAN_LIGHT, &ambient);

        // Proximity (optional)
        sensor_channel_get(dev, SENSOR_CHAN_PROX,  &prox);

        printk("[COLOR] R=%d  G=%d  B=%d  Ambient=%d  Prox=%d\n",
               r.val1, g.val1, b.val1, ambient.val1, prox.val1);

        k_msleep(300);
    }
}
