// #include <zephyr/kernel.h>
// #include "lorawan.h"
// #include "blink.h"

// /* size of stack area used by each thread */
// #define BLINK_STACKSIZE 1024
// #define LORAWAN_STACKSIZE 8192

// /* scheduling priority used by each thread */
// /* Lower priority is higher */
// #define BLINK_PRIORITY 7
// #define LORAWAN_PRIORITY 5

// // LED Blinking Handler Task
// K_THREAD_DEFINE(blink_handler_id, BLINK_STACKSIZE, blink_handler, NULL, NULL, NULL, BLINK_PRIORITY, 0, 0); // Main Green LED

// // LoRaWAN Handler Task
// K_THREAD_DEFINE(lorawan_handler_id, LORAWAN_STACKSIZE, lorawan_handler, NULL, NULL, NULL, LORAWAN_PRIORITY, 0, 0);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include "sths34pf80_zephyr.h"

/* Must match node label in your devicetree overlay */
#define STHS_NODE DT_NODELABEL(sths34pf80)

static struct sths34pf80_zephyr sths_dev;

void main(void)
{
  int ret;

  printk("STHS34PF80 human-presence demo starting...\n");

  /* Let power / I2C rails settle */
  k_msleep(4000);

  const struct i2c_dt_spec bus = I2C_DT_SPEC_GET(STHS_NODE);

  ret = sths34pf80z_init(&sths_dev, &bus);
  if (ret)
  {
    printk("STHS init failed (%d)\n", ret);
    return;
  }

  printk("STHS init OK, entering measurement loop...\n");

  while (1)
  {
    float tamb_c = 0.0f;
    float tobj_c = 0.0f;
    float tobj_comp_c = 0.0f;
    int16_t raw_tamb = 0;
    int16_t raw_tobj = 0;
    int16_t raw_tobj_comp = 0;
    int16_t raw_tpres = 0;
    int16_t raw_tmotion = 0;
    bool presence = false;
    bool motion = false;
    bool amb_shock = false;

    /* Ambient temperature */
    ret = sths34pf80z_read_ambient_raw(&sths_dev, &raw_tamb);
    if (!ret)
    {
      tamb_c = (float)raw_tamb / STHS34PF80_TAMBIENT_SENS_LSB_PER_C;
    }

    /* Object & compensated object temperature */
    ret = sths34pf80z_read_object_raw(&sths_dev, &raw_tobj);
    if (!ret)
    {
      tobj_c = (float)raw_tobj / STHS34PF80_TOBJECT_SENS_LSB_PER_C;
    }

    ret = sths34pf80z_read_tobj_comp_raw(&sths_dev, &raw_tobj_comp);
    if (!ret)
    {
      tobj_comp_c =
          (float)raw_tobj_comp / STHS34PF80_TOBJECT_SENS_LSB_PER_C;
    }

    /* Presence / motion metrics */
    ret = sths34pf80z_read_presence_metrics(&sths_dev,
                                            &presence,
                                            &motion,
                                            &amb_shock,
                                            &raw_tpres,
                                            &raw_tmotion);
    if (ret)
    {
      printk("Presence metrics read failed (%d)\n", ret);
    }

    /* Very simple heuristic:
     *   - "hot object" if compensated object temp >
     *         ambient + 3 °C
     *   - "human present" if presence flag OR hot object
     */
    bool hot_object = (tobj_comp_c > tamb_c + 3.0f);
    bool human_present = presence || hot_object;

    printk("Tambient: %.2f C (raw=%d) | "
           "Tobj: %.2f C (raw=%d) | "
           "Tobj_comp: %.2f C (raw=%d)\n",
           tamb_c, raw_tamb,
           tobj_c, raw_tobj,
           tobj_comp_c, raw_tobj_comp);

    printk("Presence flags: presence=%d motion=%d amb_shock=%d | "
           "TPRES=%d TMOTION=%d | "
           "human_present=%d hot_object=%d\n",
           presence, motion, amb_shock,
           raw_tpres, raw_tmotion,
           human_present, hot_object);

    /* For ODR = 4 Hz, 250 ms between reads is fine */
    k_msleep(250);
  }
}
