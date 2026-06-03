[![Designed by Sitron Labs](https://img.shields.io/badge/Designed_by-Sitron_Labs-FCE477.svg)](https://www.sitronlabs.com/)
[![Join the Discord community](https://img.shields.io/discord/552242187665145866.svg?logo=discord&logoColor=white&label=Discord&color=%237289da)](https://discord.gg/btnVDeWhfW)
[![PayPal Donate](https://img.shields.io/badge/PayPal-Donate-00457C.svg?logo=paypal&logoColor=white)](https://www.paypal.com/donate/?hosted_button_id=QLX8VU9Q3PFFL)
![License](https://img.shields.io/github/license/sitronlabs/SitronLabs_Pixart_PAT9130EW_Arduino_Library.svg)
![Latest Release](https://img.shields.io/github/release/sitronlabs/SitronLabs_Pixart_PAT9130EW_Arduino_Library.svg)
[![Arduino Library Manager](https://www.ardu-badge.com/badge/Sitron%20Labs%20PAT9130EW%20Arduino%20Library.svg)](https://www.ardu-badge.com/Sitron%20Labs%20PAT9130EW%20Arduino%20Library)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/sitronlabs/library/Sitron_Labs_PAT9130EW_Arduino_Library.svg)](https://registry.platformio.org/libraries/sitronlabs/Sitron_Labs_PAT9130EW_Arduino_Library)

# Sitron Labs PixArt PAT9130EW Arduino Library

Arduino library for interfacing with the PixArt PAT9130EW laser-based optical motion tracking sensor.

## Description

The PAT9130EW is a miniature optical tracking sensor that reports X/Y displacement in counts over a 3-wire SPI interface. This library provides a simple interface to detect the sensor, apply the default initialization sequence from the PixArt reference firmware, and read motion deltas.

## Installation

### Arduino IDE

Install via the Arduino Library Manager by searching for "Sitron Labs PAT9130EW".

Alternatively, install manually:
1. Download or clone this repository
2. Place it in your Arduino `libraries` folder
3. Restart the Arduino IDE

### PlatformIO

Add the library manually to your `platformio.ini` file:

```ini
lib_deps =
    https://github.com/sitronlabs/SitronLabs_Pixart_PAT9130EW_Arduino_Library.git
```

## Hardware Connections

Connect the PAT9130EW evaluation board to your Arduino using 3-wire SPI:

- VDD → 2.7 V to 3.6 V
- VDDA → 1.93 V
- GND → GND
- SCLK → SCK
- CS → Any GPIO (chip select, active low)
- SDIO → **both** MOSI and MISO (2-to-1 connection required)

The sensor uses SPI mode 3 (CPOL=1, CPHA=1) at 1 MHz by default.

Working distance (from PixArt documentation):
- 5 mm to 60 mm on glossy surfaces
- 20 mm to 40 mm on white paper

## Usage

```cpp
#include <SPI.h>
#include <pat9130ew.h>

pat9130ew sensor;

const int PIN_CS = 10;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    SPI.begin();

    if (sensor.setup(SPI, PIN_CS) != 0) {
        Serial.println("Failed to setup PAT9130EW");
        while (1) {
            delay(1000);
        }
    }

    if (sensor.detect() != 0) {
        Serial.println("PAT9130EW not detected");
        while (1) {
            delay(1000);
        }
    }

    if (sensor.init() != 0) {
        Serial.println("Failed to initialize PAT9130EW");
        while (1) {
            delay(1000);
        }
    }

    Serial.println("PAT9130EW ready");
}

void loop() {
    int16_t delta_x = 0;
    int16_t delta_y = 0;

    if (sensor.motion_available()) {
        if (sensor.delta_read(delta_x, delta_y) == 0) {
            Serial.print("deltaX: ");
            Serial.print(delta_x);
            Serial.print("\tdeltaY: ");
            Serial.println(delta_y);
        }
    }
}
```

## API Reference

### setup(SPIClass &spi_library, int pin_cs, uint32_t spi_speed)

Initializes SPI communication parameters.

- `spi_library`: SPI instance to use (typically `SPI`)
- `pin_cs`: GPIO pin for chip select
- `spi_speed`: SPI clock in Hz (default: 1000000)

Returns 0 on success, or -EINVAL if the chip select pin is invalid.

### detect(void)

Reads the product ID and verifies that it matches 0x31.

Returns 0 if detected, -1 if the product ID is wrong, or -EIO on SPI failure.

### init(void)

Performs a software reset and loads the default register settings from the PixArt reference firmware.

Returns 0 on success, or a negative error code on failure.

### register_read(pat9130ew_register reg_address, uint8_t &reg_content)

Reads an 8-bit register.

Returns 0 on success, or a negative error code on failure.

### register_write(pat9130ew_register reg_address, uint8_t reg_content)

Writes an 8-bit register.

Returns 0 on success, or a negative error code on failure.

### motion_available(void)

Returns true when the motion status bit is set in register 0x02.

### delta_read(int16_t &delta_x, int16_t &delta_y)

Reads the latest X/Y displacement in counts.

Returns 0 on success, or -EIO on SPI failure.

## Credits

The default initialization sequence in `src/pat9130ew.cpp` is derived from PixArt PAT9130EW reference firmware, licensed under [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0):

- [PAT9130EW component page](https://os.mbed.com/components/PAT9130EW-Versatile-X-Y-Motion-Tracking-/)
- [9130_referenceCode](https://os.mbed.com/teams/PixArt/code/9130_referenceCode/)
- [OTS_P9130_Demo](https://os.mbed.com/teams/PixArt/code/OTS_P9130_Demo/)
