#include <zephyr/lorawan/lorawan.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/random/random.h>

#include <stdio.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
#define LED0_NODE DT_ALIAS(led0)

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lorawan_node);

/* Customize based on network configuration */
#define LORAWAN_DEV_EUI {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x3C, 0x04}
#define LORAWAN_JOIN_EUI {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define LORAWAN_APP_KEY {0x64, 0x5A, 0xD1, 0x33, 0xEE, 0x12, 0x30, 0x2E, 0x67, 0x59, 0x9E, 0xBA, 0xC6, 0x78, 0xD0, 0x0E}

#define DELAY K_MSEC(500) // Delay between sends, 500 ms

static char data_buf[32];
static uint32_t tx_counter = 1; // start at 1

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

uint32_t mainled_rate = 150;

static int set_us915_subband(uint8_t subband /* 1..8 */)
{
    if (subband < 1 || subband > 8)
    {
        return -EINVAL;
    }

    /* LoRaMac US915 uses 6x 16-bit words for the channels mask in Zephyr */
    uint16_t mask[LORAWAN_CHANNELS_MASK_SIZE_US915] = {0};

    /* 125 kHz channels 0..63 live in mask[0..3] (16 per word).
       Each subband is 8 channels, i.e., half a 16-bit word. */
    uint8_t sb = subband - 1;
    uint8_t word = sb / 2; /* which 16-ch block (0..3) */
    bool upper_half = (sb % 2) == 1;
    mask[word] = upper_half ? 0xFF00 : 0x00FF;

    /* 500 kHz channels 64..71 live in mask[4] (bits 0..7) */
    mask[4] = BIT(sb);

    /* mask[5] remains 0 for US915 */
    return lorawan_set_channels_mask(mask, ARRAY_SIZE(mask));
}

void blink(const struct gpio_dt_spec *led, uint32_t *sleep_ms, uint32_t id)
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

void blink0(void)
{

    blink(&led0, &mainled_rate, 0);
}

static void dl_callback(uint8_t port, uint8_t data_pending,
                        int16_t rssi, int8_t snr,
                        uint8_t len, const uint8_t *data)
{
    LOG_INF("Downlink data received: ");
    for (int i = 0; i < len; i++)
        LOG_INF("%02X ", data[i]);

    LOG_INF("");
    LOG_INF("Data size: %d", len);
    LOG_INF("Data Port: %d", port);
    LOG_INF("RSSI:      %d", (int16_t)rssi);
    LOG_INF("SNR:       %d", (int16_t)snr);
    LOG_INF("Data pend: %d", data_pending);
}

// ADR change callback
static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
    uint8_t unused, max_size;

    lorawan_get_payload_sizes(&unused, &max_size);
    LOG_INF("New Datarate: DR_%d, Max Payload %d", dr, max_size);
}

/* Main program.
 */

int main(void)
{
    const struct device *lora_dev;
    struct lorawan_join_config join_cfg;
    int ret;
    
    // Using OTAA
    uint8_t dev_eui[] = LORAWAN_DEV_EUI;
    uint8_t join_eui[] = LORAWAN_JOIN_EUI;
    uint8_t app_key[] = LORAWAN_APP_KEY;
    
    k_msleep(2500);
    mainled_rate = 500; // Change led blinking rate to signal main program start.

    LOG_INF("Starting up Lora node...");

    lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
    if (!device_is_ready(lora_dev))
    {
        LOG_ERR("%s: device not ready.", lora_dev->name);
        return 0;
    }
    LOG_INF("Starting Lorawan stack...");

#if defined(CONFIG_LORAMAC_REGION_US915)
    ret = lorawan_set_region(LORAWAN_REGION_US915);
    LOG_INF("Region set to US915");
    if (ret < 0)
    {
        LOG_ERR("lorawan_set_region failed: %d", ret);
        return 0;
    }
#endif

    lorawan_enable_adr(true);
    ret = lorawan_start();
    if (ret < 0)
    {
        LOG_ERR("lorawan_start failed: %d", ret);
        return 0;
    }

    // Enable callbacks
    struct lorawan_downlink_cb downlink_cb = {
        .port = LW_RECV_PORT_ANY,
        .cb = dl_callback};

    lorawan_register_downlink_callback(&downlink_cb);
    lorawan_register_dr_changed_callback(lorwan_datarate_changed);

    uint32_t random = sys_rand32_get();

    uint16_t dev_nonce = random & 0x0000FFFF;

    // Using OTAA
    join_cfg.mode = LORAWAN_ACT_OTAA;
    join_cfg.dev_eui = dev_eui;
    join_cfg.otaa.join_eui = join_eui;
    join_cfg.otaa.app_key = app_key;
    join_cfg.otaa.nwk_key = app_key;
    join_cfg.otaa.dev_nonce = dev_nonce;

    LOG_INF("Joining TTN network over OTAA");

    int err = set_us915_subband(2); /* e.g., subband 2 (channels 8..15) */
    if (err)
    {
        printk("subband set failed: %d\n", err);
    }

    // Loop until we connect
    do
    {
        ret = lorawan_join(&join_cfg);
        if (ret < 0)
        {
            LOG_ERR("lorawan_join_network failed: %d", ret);
            mainled_rate = 100; // Flash the led very rapidly to signal failure.
            LOG_INF("Sleeping for 5s to try again to join network.");
            k_sleep(K_MSEC(5000));
            mainled_rate = 0;
        }
    } while (ret < 0);

    lorawan_enable_adr(false);          // stop the network from holding you at DR_0
    lorawan_set_datarate(LORAWAN_DR_4); // US915 DR_4
    LOG_INF("Lorawan join succeeded");
    LOG_INF("Starting to send data");

    while (1)
    {
        const uint8_t port = 2;

        // Build "helloworld <n>" into data_buf
        uint8_t len = (uint8_t)snprintf(
            data_buf, sizeof(data_buf),
            "helloworld %lu", (unsigned long)tx_counter++);

        if (len >= sizeof(data_buf))
        {
            len = sizeof(data_buf) - 1;
        }

        // Try to send; if MAC cmds make it too tight, flush once and retry
        int ret = 0;
        for (int tries = 0; tries < 1; ++tries)
        {
            ret = lorawan_send(port, data_buf, len, LORAWAN_MSG_UNCONFIRMED);
            if (ret == -EAGAIN)
            {
                (void)lorawan_send(0, NULL, 0, LORAWAN_MSG_UNCONFIRMED); // MAC-only uplink to flush
                k_sleep(K_SECONDS(2));
                continue;
            }
            break;
        }

        if (ret < 0)
        {
            LOG_ERR("lorawan_send failed: %d", ret);
            k_sleep(DELAY);
            continue;
        }

        LOG_INF("Data sent!");
        LOG_INF("Sent: \"%s\" (len=%u)\n\n", data_buf, len);
        k_sleep(DELAY);
    }

    return 0;
}

// Task for handling blinking leds.
K_THREAD_DEFINE(blink0_id, STACKSIZE, blink0, NULL, NULL, NULL, PRIORITY, 0, 0); // Main Green LED

// Lorawan handling task to join the TTN network
// K_THREAD_DEFINE(lorawan_task_id, STACKSIZE, lorawan_task, NULL, NULL, NULL, PRIORITY, 0, 0);