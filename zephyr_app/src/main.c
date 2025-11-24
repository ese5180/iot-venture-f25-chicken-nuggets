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

// #include <zephyr/kernel.h>
// #include <zephyr/device.h>
// #include <zephyr/devicetree.h>
// #include <zephyr/sys/printk.h>

// #include "sths34pf80_reg.h"

// /* Must match node label in your devicetree overlay */
// #define STHS_NODE DT_NODELABEL(sths34pf80)

// static struct sths34pf80_zephyr sths_dev;

// void main(void)
// {
//   int ret;

//   printk("STHS34PF80 human-presence demo starting...\n");

//   /* Let power / I2C rails settle */
//   k_msleep(4000);

//   const struct i2c_dt_spec bus = I2C_DT_SPEC_GET(STHS_NODE);

//   ret = sths34pf80z_init(&sths_dev, &bus);
//   if (ret)
//   {
//     printk("STHS init failed (%d)\n", ret);
//     return;
//   }

//   printk("STHS init OK, entering measurement loop...\n");

//   while (1)
//   {
//     float tamb_c = 0.0f;
//     float tobj_c = 0.0f;
//     float tobj_comp_c = 0.0f;
//     int16_t raw_tamb = 0;
//     int16_t raw_tobj = 0;
//     int16_t raw_tobj_comp = 0;
//     int16_t raw_tpres = 0;
//     int16_t raw_tmotion = 0;
//     bool presence = false;
//     bool motion = false;
//     bool amb_shock = false;

//     /* Ambient temperature */
//     ret = sths34pf80z_read_ambient_raw(&sths_dev, &raw_tamb);
//     if (!ret)
//     {
//       tamb_c = (float)raw_tamb / STHS34PF80_TAMBIENT_SENS_LSB_PER_C;
//     }

//     /* Object & compensated object temperature */
//     ret = sths34pf80z_read_object_raw(&sths_dev, &raw_tobj);
//     if (!ret)
//     {
//       tobj_c = (float)raw_tobj / STHS34PF80_TOBJECT_SENS_LSB_PER_C;
//     }

//     ret = sths34pf80z_read_tobj_comp_raw(&sths_dev, &raw_tobj_comp);
//     if (!ret)
//     {
//       tobj_comp_c =
//           (float)raw_tobj_comp / STHS34PF80_TOBJECT_SENS_LSB_PER_C;
//     }

//     /* Presence / motion metrics */
//     ret = sths34pf80z_read_presence_metrics(&sths_dev,
//                                             &presence,
//                                             &motion,
//                                             &amb_shock,
//                                             &raw_tpres,
//                                             &raw_tmotion);
//     if (ret)
//     {
//       printk("Presence metrics read failed (%d)\n", ret);
//     }

//     /* Very simple heuristic:
//      *   - "hot object" if compensated object temp >
//      *         ambient + 3 °C
//      *   - "human present" if presence flag OR hot object
//      */
//     bool hot_object = (tobj_comp_c > tamb_c + 3.0f);
//     bool human_present = presence || hot_object;

//     printk("Tambient: %.2f C (raw=%d) | "
//            "Tobj: %.2f C (raw=%d) | "
//            "Tobj_comp: %.2f C (raw=%d)\n",
//            tamb_c, raw_tamb,
//            tobj_c, raw_tobj,
//            tobj_comp_c, raw_tobj_comp);

//     printk("Presence flags: presence=%d motion=%d amb_shock=%d | "
//            "TPRES=%d TMOTION=%d | "
//            "human_present=%d hot_object=%d\n",
//            presence, motion, amb_shock,
//            raw_tpres, raw_tmotion,
//            human_present, hot_object);

//     /* For ODR = 4 Hz, 250 ms between reads is fine */
//     k_msleep(250);
//   }
// }

// #include <zephyr/kernel.h>
// #include <zephyr/device.h>
// #include <zephyr/devicetree.h>
// #include <zephyr/drivers/i2c.h>
// #include <zephyr/sys/printk.h>

// #include "sths34pf80_reg.h"

// /* Node label from the overlay */
// #define STHS_NODE DT_NODELABEL(sths34pf80)

// /* Get I2C bus + address from devicetree */
// static const struct i2c_dt_spec sths_i2c = I2C_DT_SPEC_GET(STHS_NODE);

// /* ST driver context */
// static stmdev_ctx_t sths_ctx;

// /* --- Conversion factors (check datasheet and adjust if needed!) --- */
// /* These are placeholders – verify against the STHS34PF80 datasheet. */
// #define STHS34PF80_TAMBIENT_LSB_PER_C 10.0f /* TODO: verify */
// #define STHS34PF80_TOBJECT_LSB_PER_C 200.0f /* TODO: verify */

// /* --- Zephyr <-> ST driver glue --- */

// static int32_t sths_platform_write(void *handle,
//                                    uint8_t reg,
//                                    const uint8_t *bufp,
//                                    uint16_t len)
// {
//   const struct i2c_dt_spec *spec = handle;

//   /* Compose [reg | data...] buffer on stack */
//   uint8_t tmp[1 + 32];

//   if (len > 32)
//   {
//     /* This device uses small registers; guard against silly sizes. */
//     return -1;
//   }

//   tmp[0] = reg;
//   for (uint16_t i = 0; i < len; i++)
//   {
//     tmp[1 + i] = bufp[i];
//   }

//   int ret = i2c_write_dt(spec, tmp, 1 + len);
//   return (ret == 0) ? 0 : -1;
// }

// static int32_t sths_platform_read(void *handle,
//                                   uint8_t reg,
//                                   uint8_t *bufp,
//                                   uint16_t len)
// {
//   const struct i2c_dt_spec *spec = handle;

//   int ret = i2c_write_read_dt(spec, &reg, 1, bufp, len);
//   return (ret == 0) ? 0 : -1;
// }

// static void sths_platform_delay(uint32_t ms)
// {
//   k_msleep(ms);
// }

// /* Helper to print a float as X.XX without needing printf float support */
// static void print_temp(const char *label, float temp_c, int16_t raw)
// {
//   int32_t t100 = (int32_t)(temp_c * 100.0f);
//   int32_t whole = t100 / 100;
//   int32_t frac = t100 % 100;

//   if (frac < 0)
//   {
//     frac = -frac;
//   }

//   printk("%s: %d.%02d C (raw=%d)\n",
//          label,
//          (int)whole,
//          (int)frac,
//          raw);
// }

// /* Optional: quick I2C scan on i2c1 for sanity */
// static void i2c_scan_bus(const struct i2c_dt_spec *spec)
// {
//   printk("I2C scan on %s:\n", spec->bus->name);

//   for (uint8_t addr = 0x03; addr < 0x78; addr++)
//   {
//     struct i2c_msg msg = {
//         .buf = NULL,
//         .len = 0,
//         .flags = I2C_MSG_WRITE | I2C_MSG_STOP,
//     };

//     int ret = i2c_transfer(spec->bus, &msg, 1, addr);
//     if (ret == 0)
//     {
//       printk("  Found device at 0x%02x\n", addr);
//     }
//   }

//   printk("Scan done on %s.\n", spec->bus->name);
// }

// /* --- MAIN --- */

// void main(void)
// {
//   int32_t ret;

//   printk("STHS34PF80 demo using ST driver (non-NS / I2C1) starting...\n");

//   if (!device_is_ready(sths_i2c.bus))
//   {
//     printk("I2C device %s not ready!\n", sths_i2c.bus->name);
//     return;
//   }

//   /* Let rails settle a bit */
//   k_msleep(4000);

//   /* Optional: I2C scan for sanity */
//   i2c_scan_bus(&sths_i2c);

//   printk("STHS node bound to bus %s, addr 0x%02x\n",
//          sths_i2c.bus->name, sths_i2c.addr);

//   /* Hook Zephyr I2C into ST driver context */
//   sths_ctx.handle = (void *)&sths_i2c;
//   sths_ctx.write_reg = sths_platform_write;
//   sths_ctx.read_reg = sths_platform_read;
//   sths_ctx.mdelay = sths_platform_delay;

//   /* Check WHO_AM_I via ST driver */
//   uint8_t who = 0;
//   ret = sths34pf80_device_id_get(&sths_ctx, &who);
//   if (ret != 0)
//   {
//     printk("device_id_get failed (%ld)\n", (long)ret);
//     return;
//   }

//   printk("STHS34PF80 WHO_AM_I = 0x%02x\n", who);

//   if (who != STHS34PF80_ID)
//   {
//     printk("Unexpected WHO_AM_I (0x%02x, expected 0x%02x)\n",
//            who, STHS34PF80_ID);
//     return;
//   }

//   /* Basic config: BDU, ODR, averages, gain */
//   /* Block data update */
//   (void)sths34pf80_block_data_update_set(&sths_ctx, 1);

//   /* Temperature averaging (ambient + object) */
//   (void)sths34pf80_avg_tambient_num_set(&sths_ctx, STHS34PF80_AVG_T_4);
//   (void)sths34pf80_avg_tobject_num_set(&sths_ctx, STHS34PF80_AVG_TMOS_32);

//   /* Gain mode (default) */
//   (void)sths34pf80_gain_mode_set(&sths_ctx, STHS34PF80_GAIN_DEFAULT_MODE);

//   /* Output data rate: 4 Hz */
//   (void)sths34pf80_odr_set(&sths_ctx, STHS34PF80_ODR_AT_4Hz);

//   printk("STHS config done, entering measurement loop...\n");

//   while (1)
//   {
//     int16_t raw_tamb = 0;
//     int16_t raw_tobj = 0;
//     int16_t raw_tobj_comp = 0;
//     int16_t raw_tpres = 0;
//     int16_t raw_tmotion = 0;

//     /* Read raw values via ST API */
//     (void)sths34pf80_tambient_raw_get(&sths_ctx, &raw_tamb);
//     (void)sths34pf80_tobject_raw_get(&sths_ctx, &raw_tobj);
//     (void)sths34pf80_tobj_comp_raw_get(&sths_ctx, &raw_tobj_comp);
//     (void)sths34pf80_tpresence_raw_get(&sths_ctx, &raw_tpres);
//     (void)sths34pf80_tmotion_raw_get(&sths_ctx, &raw_tmotion);

//     /* Convert to °C with placeholder scales – adjust per datasheet */
//     float tamb_c = raw_tamb / STHS34PF80_TAMBIENT_LSB_PER_C;
//     float tobj_c = raw_tobj / STHS34PF80_TOBJECT_LSB_PER_C;
//     float tobj_comp_c = raw_tobj_comp / STHS34PF80_TOBJECT_LSB_PER_C;

//     /* Simple heuristic: hot object if Tobj_comp > Tamb + 3°C */
//     bool hot_object = (tobj_comp_c > tamb_c + 3.0f);

//     print_temp("Tambient", tamb_c, raw_tamb);
//     print_temp("Tobject", tobj_c, raw_tobj);
//     print_temp("Tobject_comp", tobj_comp_c, raw_tobj_comp);

//     printk("Presence/motion raw: TPRES=%d TMOTION=%d | hot_object=%d\n",
//            raw_tpres, raw_tmotion, hot_object);

//     /* For 4 Hz ODR, ~250 ms between reads is fine */
//     k_msleep(250);
//   }
// }

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sths34pf80_reg.h"
#include "color.h"
#include "pir.h"
#include "lorawan.h"

#include <stdint.h>

/* I2C bus used for both STHS34PF80 and APDS9960 */
#define I2C1_NODE DT_NODELABEL(i2c1)
#define STHS34PF80_I2C_ADDR 0x5A

#define LORAWAN_STACKSIZE 8192
#define LORAWAN_PRIORITY 5

/* Temperature conversion factors (LSB per °C).
 * Adjust if you want exact scaling from the datasheet. */
#define STHS34PF80_TAMBIENT_SENS_LSB_PER_C 10.0f
#define STHS34PF80_TOBJECT_SENS_LSB_PER_C 10.0f

#define PIR_STACKSIZE 1024
#define PIR_PRIORITY 6

uint32_t mainled_rate = 0;

/* Global context for STHS34PF80 */
static const struct device *i2c1_dev;
static stmdev_ctx_t sths_ctx;

char print_buffer[256];

// LoRaWAN Handler Task
K_THREAD_DEFINE(lorawan_handler_id, LORAWAN_STACKSIZE, lorawan_handler, NULL, NULL, NULL, LORAWAN_PRIORITY, 0, 0);

// PIR handler
K_THREAD_DEFINE(pir_handler_id, PIR_STACKSIZE, pir_handler, NULL, NULL, NULL, PIR_PRIORITY, 0, 0);

/* ---------- Low-level I2C bridge for ST driver ---------- */

static int32_t sths_i2c_write(void *ctx, uint8_t reg,
                              const uint8_t *bufp, uint16_t len)
{
  const struct device *i2c = (const struct device *)ctx;

  /* Build [reg | data...] buffer on stack */
  uint8_t tmp[1 + 32];
  if (len > 32U)
  {
    /* Avoid silly-sized transfers; tune if needed */
    return -1;
  }

  tmp[0] = reg;
  memcpy(&tmp[1], bufp, len);

  int ret = i2c_write(i2c, tmp, len + 1, STHS34PF80_I2C_ADDR);
  return (ret == 0) ? 0 : -1;
}

static int32_t sths_i2c_read(void *ctx, uint8_t reg,
                             uint8_t *bufp, uint16_t len)
{
  const struct device *i2c = (const struct device *)ctx;

  int ret = i2c_write_read(i2c,
                           STHS34PF80_I2C_ADDR,
                           &reg, 1,
                           bufp, len);
  return (ret == 0) ? 0 : -1;
}

static void sths_delay(uint32_t msec)
{
  k_msleep(msec);
}

/* ---------- STHS34PF80 initialization ---------- */

static int sths_init(void)
{
  int32_t ret;
  uint8_t whoami = 0;

  i2c1_dev = DEVICE_DT_GET(I2C1_NODE);
  if (!device_is_ready(i2c1_dev))
  {
    printk("I2C1 device not ready\n");
    return -ENODEV;
  }

  /* Wire ST driver context to Zephyr I2C */
  sths_ctx.write_reg = sths_i2c_write;
  sths_ctx.read_reg = sths_i2c_read;
  sths_ctx.mdelay = sths_delay;
  sths_ctx.handle = (void *)i2c1_dev;

  /* Check WHOAMI */
  ret = sths34pf80_device_id_get(&sths_ctx, &whoami);
  if (ret != 0)
  {
    printk("STHS34PF80: WHOAMI read failed (%d)\n", (int)ret);
    return -EIO;
  }

  printk("STHS34PF80 WHOAMI=0x%02x\n", whoami);
  if (whoami != STHS34PF80_ID)
  {
    printk("STHS34PF80: unexpected ID (expected 0x%02x)\n", STHS34PF80_ID);
    return -EINVAL;
  }

  /* Block data update */
  (void)sths34pf80_block_data_update_set(&sths_ctx, 1);

  /* Set averages:
   *  - Object temperature: use 32 samples (enum uses TMOS naming)
   *  - Ambient temperature: use 8 samples (T_8 etc.)
   */
  (void)sths34pf80_avg_tobject_num_set(&sths_ctx, STHS34PF80_AVG_TMOS_32);
  (void)sths34pf80_avg_tambient_num_set(&sths_ctx, STHS34PF80_AVG_T_8);

  /* Gain mode: use default gain */
  (void)sths34pf80_gain_mode_set(&sths_ctx, STHS34PF80_GAIN_DEFAULT_MODE);

  /* Output data rate: 4 Hz */
  (void)sths34pf80_odr_set(&sths_ctx, STHS34PF80_ODR_AT_4Hz);

  return 0;
}

/* ---------- APDS9960 thread wrapper ---------- */

/* color_handler() is defined in color.c and runs its own while(1) loop.
 * We put it in its own Zephyr thread so main() can still run STHS34PF80. */
static void color_thread_entry(void *a, void *b, void *c)
{
  ARG_UNUSED(a);
  ARG_UNUSED(b);
  ARG_UNUSED(c);
  color_handler(); /* never returns */
}

/* Stack size / priority: tweak if needed */
K_THREAD_DEFINE(color_thread_id,
                2048,
                color_thread_entry,
                NULL, NULL, NULL,
                5, 0, 0);

/* ---------- Main ---------- */

void main(void)
{
  int ret;

  printk("STHS34PF80 + APDS9960 demo starting...\n");

  /* Give power rails & APDS reset some time */
  k_msleep(1000);

  ret = sths_init();
  if (ret)
  {
    printk("STHS34PF80 init failed (%d), continuing anyway\n", ret);
  }
  else
  {
    printk("STHS34PF80 init OK, entering measurement loop...\n");
  }

  while (1)
  {
    /* Only try to read if init succeeded */
    if (ret == 0)
    {
      sths34pf80_drdy_status_t drdy;
      int32_t st = sths34pf80_drdy_status_get(&sths_ctx, &drdy);

      if (st == 0 && drdy.drdy)
      {
        int16_t raw_tamb = 0;
        int16_t raw_tobj = 0;
        int16_t raw_tobj_comp = 0;
        int16_t raw_tpres = 0;
        int16_t raw_tmotion = 0;
        sths34pf80_func_status_t fstatus;

        (void)sths34pf80_tambient_raw_get(&sths_ctx, &raw_tamb);
        (void)sths34pf80_tobject_raw_get(&sths_ctx, &raw_tobj);
        (void)sths34pf80_tobj_comp_raw_get(&sths_ctx, &raw_tobj_comp);
        (void)sths34pf80_tpresence_raw_get(&sths_ctx, &raw_tpres);
        (void)sths34pf80_tmotion_raw_get(&sths_ctx, &raw_tmotion);
        (void)sths34pf80_func_status_get(&sths_ctx, &fstatus);

        float tamb_c = (float)raw_tamb / STHS34PF80_TAMBIENT_SENS_LSB_PER_C;
        float tobj_c = (float)raw_tobj / STHS34PF80_TOBJECT_SENS_LSB_PER_C;
        float tobj_comp_c = (float)raw_tobj_comp / STHS34PF80_TOBJECT_SENS_LSB_PER_C;

        bool presence = (fstatus.pres_flag != 0U);
        bool motion = (fstatus.mot_flag != 0U);
        bool amb_shock = (fstatus.mot_flag != 0U);

        bool hot_object = (tobj_comp_c > tamb_c + 3.0f);
        bool human_present = presence || hot_object;

        printk("STHS34PF80: Tamb=%.2f C Tobj=%.2f C Tobj_comp=%.2f C | "
               "TPRES=%d TMOTION=%d | flags: pres=%d mot=%d shock=%d | "
               "human_present=%d hot_object=%d\n",
               tamb_c, tobj_c, tobj_comp_c,
               raw_tpres, raw_tmotion,
               presence, motion, amb_shock,
               human_present, hot_object);

        int32_t t100 = (int32_t)(tobj_comp_c * 100.0f); /* °C * 100 */
        int32_t t_whole = t100 / 100;
        int32_t t_frac = t100 % 100;
        if (t_frac < 0)
        {
          t_frac = -t_frac;
        }

        /* PIR status: "M" = motion, "N" = no motion */
        const char *pir_str = motion ? "M" : "N";

        /* Populate global print_buffer */
        snprintf(print_buffer, sizeof(print_buffer),
                 "Tobj=%ld.%02ldC,PIR=%s,HP=%d",
                 (long)t_whole,
                 (long)t_frac,
                 pir_str,
                 human_present ? 1 : 0);
      }
    }

    /* 4 Hz ODR: 250 ms between polls is fine */
    k_msleep(250);
  }
}
