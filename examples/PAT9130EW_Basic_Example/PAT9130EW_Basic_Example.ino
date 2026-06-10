/*
 * PAT9130EW Basic Example
 *
 * This example demonstrates basic usage of the PAT9130EW optical motion tracking sensor. It initializes the sensor, then prints deltaX and deltaY counts whenever motion is detected.
 *
 * Hardware Connections:
 * - VDD to 3.3 V
 * - GND to GND
 * - SCLK to SCK
 * - CS to pin 10 (or change PIN_CS below)
 * - SDIO to MOSI through a 3.3k resistor and to MISO directly (3-wire SPI)
 *
 * Author: Sitron Labs
 * Date: 2026
 */

#include <SPI.h>
#include <pat9130ew.h>

/* PAT9130EW instance */
pat9130ew sensor;

/* Configuration parameters */
const int PIN_CS = 10;

void setup() {
    /* Initialize serial communication */
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("PAT9130EW Basic Example");
    Serial.println("=======================");

    /* Initialize SPI */
    SPI.begin();

    /* Setup PAT9130EW */
    int res = sensor.setup(SPI, PIN_CS);
    if (res < 0) {
        Serial.println("Failed to setup PAT9130EW!");
        while (1) {
            delay(1000);
        }
    }

    /* Detect the sensor */
    res = sensor.detect();
    if (res < 0) {
        Serial.println("PAT9130EW not detected! Check connections.");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("PAT9130EW detected successfully!");

    /* Initialize the sensor */
    res = sensor.init();
    if (res < 0) {
        Serial.println("Failed to initialize PAT9130EW!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("PAT9130EW initialized.");
    Serial.println("Move the sensor over a surface to see motion counts.");
}

void loop() {
    int16_t delta_x = 0;
    int16_t delta_y = 0;
    bool x_overflow = false;
    bool y_overflow = false;
    static int32_t total_x = 0;
    static int32_t total_y = 0;

    if (sensor.motion_delta_read(delta_x, delta_y, x_overflow, y_overflow) == 1) {
        if ((delta_x != 0) || (delta_y != 0)) {
            total_x += delta_x;
            total_y += delta_y;

            Serial.print("deltaX: ");
            Serial.print(delta_x);
            Serial.print("\tdeltaY: ");
            Serial.println(delta_y);

            Serial.print("X-axis Counts: ");
            Serial.print(total_x);
            Serial.print("\tY-axis Counts: ");
            Serial.println(total_y);

            if (x_overflow || y_overflow) {
                Serial.println("Warning: motion buffer overflow");
            }
        }
    }
}
