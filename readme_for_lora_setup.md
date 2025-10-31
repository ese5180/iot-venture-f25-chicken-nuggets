### Steps for setting LoRa up:

###### 1. Solder an antenna or attach some other antenna to RFM95
I used a 7.8cm wire as described [here](https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/assembly) (This tutorial is fantastic overall for the RFM95)

###### 2. Pin connections:

First, I connect a 3.3V power supply to VIO_REF on the nRF7002DK (tie GND of power supply to nRF7002DK GND) - this enables 3.3V GPIO outputs from the GPIO pins

So from the RFM95 board I have:

VIN -> VDD

GND -> GND

EN -> floating, no connection

G0 -> LoRa DIO GPIO0 -> Pin 1.13

SCK -> SPI SCK -> Pin 1.12

MISO -> SPI MISO -> Pin 1.10

MOSI -> SPI MOSI -> Pin 1.11

CS -> SPI CS -> Pin 1.3

RST -> LoRa SX1276 Reset -> Pin 1.15

G1 -> LoRa DIO GPIO1 -> Pin 1.14


This is the same thing just in a table
(**JUST LOOK AT THIS TABLE**)
| **RFM95** | **Device Tree**        | **nRF7002DK**     |
|------------|------------------------|-------------------|
| VIN        | —                      | VDD               |
| GND        | —                      | GND               |
| EN         | —                      | (Floating)        |
| G0         | LoRa DIO GPIO0         | Pin 1.13          |
| SCK        | SPI SCK                | Pin 1.12          |
| MISO       | SPI MISO               | Pin 1.10          |
| MOSI       | SPI MOSI               | Pin 1.11          |
| CS         | SPI CS                 | Pin 1.3           |
| RST        | LoRa SX1276 Reset      | Pin 1.15          |
| G1         | LoRa DIO GPIO1         | Pin 1.14          |

I chose these pins because according to the [pin mapping of nRF7002DK](https://docs.nordicsemi.com/bundle/ug_nrf7002_dk/page/UG/nrf7002_DK/connector_if.html) they don't seem to conflict with any other functions

###### 3. TTN Setup
Then, I set up a device on TTN according to the instructions in the Lab 0 writeup.
The Dev EUI and App Key are hardcoded into `main.c`. These are from the device dashboard in TTN (End Devices > (Your-Device) > Settings > Basic & Network Layer). You also get them when you first create and add a device. I think for OTAA only the Dev EUI and App Key are needed. I left the Join EUI in `main.c` as 

```
#define LORAWAN_JOIN_EUI		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
```

And it seems to work perfectly fine.

The transmissions seem to be really unreliable (One will succeed every ~10-15 or so), I'm unsure why at the moment. It could be that my antenna sucks. The "helloworld" transmissions do show up in the TTN dashboard though.

Most likely we will still with using OTAA for now as it seems to work. I had trouble getting ABP to work, but OTAA is more advanced/secure anyways.


I took the code in `main.c` (I don't think it's very different from the zephyr LoRa class A OTAA sample, there are a couple of modifications) from a sample written [here](https://github.com/fcgdam/zLorawan_Node/blob/master/src/main.c), and whoever wrote this also wrote a very helpful [blog post](https://primalcortex.wordpress.com/2020/11/17/a-zephyr-rtos-based-ttn-lorawan-node/) where I got the device tree from.

Some other helpful links:
- [Wiring diagram which doesn't exactly line up with ours but it's nice](https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/arduino-wiring)
- [Explanation of RFM95 Pins](https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/pinouts)
- [Pin mapping of nRF7002DK](https://docs.nordicsemi.com/bundle/ug_nrf7002_dk/page/UG/nrf7002_DK/connector_if.html)

Links for myself
- [set datarate issue](https://github.com/zephyrproject-rtos/zephyr/issues/31551)
- [lorawan.h](https://docs.zephyrproject.org/apidoc/latest/lorawan_8h_source.html)
- [some bug report on Mlmeconfirm](https://github.com/zephyrproject-rtos/zephyr/issues/36953)
- [ttn dashboard](https://nam1.cloud.thethings.network/console/applications/ese518-iot-venture-chicken-nuggies/devices/jason-nrf7002-dk-rfm95)
- [radiolib?](https://github.com/jgromes/RadioLib)
- [unable to join network rx2 timeout](https://www.thethingsnetwork.org/forum/t/unable-to-join-network-rx-2-timeout/68603/18)
