/* Self header */
#include "pat9130ew.h"

/**
 * Default register settings from PixArt PAT9130EW reference firmware.
 *
 * Derived from the "initialize" table in registerArrays.h (9130_referenceCode),
 * Apache License 2.0. See README.md for source links.
 */
static const uint8_t PAT9130EW_INIT_SEQUENCE[][2] = {
    {0x7F, 0x00},
    {0x05, 0xA8},
    {0x09, 0x5A},
    {0x51, 0x06},
    {0x0D, 0x1F},
    {0x0E, 0x1F},
    {0x07, 0x00},
    {0x1B, 0x42},
    {0x2E, 0x40},
    {0x32, 0x40},
    {0x33, 0x02},
    {0x34, 0x00},
    {0x36, 0xE0},
    {0x38, 0xA0},
    {0x39, 0x01},
    {0x3E, 0x14},
    {0x44, 0x02},
    {0x4A, 0xE0},
    {0x4F, 0x02},
    {0x52, 0x0D},
    {0x57, 0x03},
    {0x59, 0x03},
    {0x5B, 0x03},
    {0x5C, 0xFF},
    {0x7F, 0x01},
    {0x00, 0x25},
    {0x07, 0x78},
    {0x20, 0x00},
    {0x21, 0x40},
    {0x23, 0x00},
    {0x2F, 0x64},
    {0x37, 0x30},
    {0x3B, 0x64},
    {0x43, 0x0A},
    {0x59, 0x01},
    {0x5A, 0x01},
    {0x5C, 0x04},
    {0x5E, 0x04},
    {0x7F, 0x02},
    {0x51, 0x02},
    {0x7F, 0x03},
    {0x05, 0x0C},
    {0x06, 0x0C},
    {0x07, 0x0C},
    {0x08, 0x0C},
    {0x7F, 0x04},
    {0x05, 0x01},
    {0x53, 0x08},
    {0x7F, 0x05},
    {0x00, 0x02},
    {0x09, 0x01},
    {0x0A, 0x1C},
    {0x0B, 0x24},
    {0x0C, 0x1C},
    {0x0D, 0x24},
    {0x12, 0x08},
    {0x14, 0x02},
    {0x16, 0x02},
    {0x18, 0x1C},
    {0x19, 0x24},
    {0x1A, 0x1C},
    {0x1B, 0x24},
    {0x20, 0x08},
    {0x22, 0x02},
    {0x24, 0x02},
    {0x26, 0x88},
    {0x2F, 0x7C},
    {0x30, 0x07},
    {0x3D, 0x00},
    {0x3E, 0x98},
    {0x7F, 0x06},
    {0x34, 0x03},
    {0x7F, 0x07},
    {0x00, 0x01},
    {0x02, 0xC4},
    {0x03, 0x13},
    {0x06, 0x0C},
    {0x0F, 0x0A},
    {0x14, 0x02},
    {0x35, 0x39},
    {0x36, 0x3F},
    {0x46, 0x03},
    {0x47, 0x0F},
    {0x4B, 0x97},
    {0x7F, 0x00},
    {0x09, 0x00},
};

static const size_t PAT9130EW_INIT_SEQUENCE_LENGTH = sizeof(PAT9130EW_INIT_SEQUENCE) / sizeof(PAT9130EW_INIT_SEQUENCE[0]);

/**
 * Initialize the PAT9130EW sensor with SPI communication parameters
 *
 * Stores the SPI instance, chip select pin, and clock speed for subsequent
 * operations. The SDIO pin on the sensor must be wired to both MOSI and MISO.
 *
 * @param[in] spi_library Reference to the SPIClass instance to use
 * @param[in] pin_cs GPIO pin connected to the chip select (active low)
 * @param[in] spi_speed SPI clock speed in Hz
 * @return 0 on success, -EINVAL if the chip select pin is invalid
 */
int pat9130ew::setup(SPIClass &spi_library, const int pin_cs, const uint32_t spi_speed) {

    /* Ensure chip select pin is valid */
    if (pin_cs < 0) {
        return -EINVAL;
    }

    /* Store SPI instance, chip select pin, and clock speed */
    m_spi_library = &spi_library;
    m_pin_cs = pin_cs;
    m_spi_speed = spi_speed;

    /* Set chip select pin as output and pull it high */
    pinMode(m_pin_cs, OUTPUT);
    digitalWrite(m_pin_cs, HIGH);

    /* Return success */
    return 0;
}

/**
 * Reads the contents of the given register.
 * @param[in] reg_address The address of the register.
 * @param[out] reg_content Variable that will be updated with the contents of the register.
 * @return 0 in case of success, or a negative error code otherwise.
 */
int pat9130ew::register_read(const enum pat9130ew_register reg_address, uint8_t &reg_content) {

    /* Ensure library has been configured */
    if ((m_spi_library == NULL) || (m_pin_cs < 0)) {
        return -EINVAL;
    }

    m_spi_library->beginTransaction(SPISettings(m_spi_speed, MSBFIRST, SPI_MODE3));
    uint8_t address;
    address = ((uint8_t)reg_address) & 0x7F;
    digitalWrite(m_pin_cs, LOW);
    m_spi_library->transfer(address);
    delayMicroseconds(1);
    reg_content = m_spi_library->transfer(0x00);
    digitalWrite(m_pin_cs, HIGH);

    m_spi_library->endTransaction();

    /* Return success */
    return 0;
}

/**
 * Updates the content of the given register.
 * @param[in] reg_address The address of the register.
 * @param[in] reg_content The new content of the register.
 * @return 0 in case of success, or a negative error code otherwise.
 */
int pat9130ew::register_write(const enum pat9130ew_register reg_address, const uint8_t reg_content) {
    uint8_t address;

    /* Ensure library has been configured */
    if ((m_spi_library == NULL) || (m_pin_cs < 0)) {
        return -EINVAL;
    }

    m_spi_library->beginTransaction(SPISettings(m_spi_speed, MSBFIRST, SPI_MODE3));

    address = ((uint8_t)reg_address) | 0x80;
    digitalWrite(m_pin_cs, LOW);
    m_spi_library->transfer(address);
    m_spi_library->transfer(reg_content);
    digitalWrite(m_pin_cs, HIGH);

    m_spi_library->endTransaction();

    /* Return success */
    return 0;
}

/**
 * Detect and verify the presence of a PAT9130EW sensor
 *
 * Reads the product ID register and checks for the expected value (0x31).
 *
 * @return 0 if a valid PAT9130EW device is detected, -EIO on SPI communication
 *         failure, or -1 if the product ID does not match
 */
int pat9130ew::detect(void) {
    int res;

    /* Read product ID */
    uint8_t product_id = 0x00;
    res = register_read(PAT9130EW_REGISTER_PRODUCT_ID, product_id);
    if (res < 0) {
        return -EIO;
    }
    if (product_id != 0x31) {
        return -1;
    }

    /* Return success */
    return 0;
}

/**
 * Initialize the sensor (software reset and default register settings)
 *
 * Issues a software reset, waits for the device to recover, then loads the
 * default register settings from the PixArt reference firmware.
 *
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::init(void) {
    int res;

    /* Issue software reset */
    res = register_write(PAT9130EW_REGISTER_CONFIGURATION, 0x80);
    if (res < 0) {
        return -EIO;
    }
    delay(1);

    /* Ensure sensor is detected */
    res = detect();
    if (res < 0) {
        return res;
    }

    /* Load default register settings */
    for (size_t index = 0; index < PAT9130EW_INIT_SEQUENCE_LENGTH; index++) {
        res = register_write((enum pat9130ew_register)PAT9130EW_INIT_SEQUENCE[index][0], PAT9130EW_INIT_SEQUENCE[index][1]);
        if (res < 0) {
            return -EIO;
        }
    }

    /* Return success */
    return 0;
}

/**
 * Check whether new motion data is available
 *
 * Reads the motion status register and returns whether bit 7 is set.
 *
 * @return true if motion data is available, false otherwise or on read failure
 */
bool pat9130ew::motion_available(void) {

    /* Read motion status */
    uint8_t motion_status = 0x00;
    if (register_read(PAT9130EW_REGISTER_MOTION, motion_status) < 0) {
        return false;
    }

    /* Return true if motion status is set */
    return (motion_status & 0x80) != 0;
}

/**
 * Read the latest X/Y displacement since the previous report
 *
 * Reads the low and high delta registers and combines them into signed 16-bit
 * displacement values, matching the PixArt reference firmware format.
 *
 * @param[out] delta_x Variable that will receive the X displacement in counts
 * @param[out] delta_y Variable that will receive the Y displacement in counts
 * @return 0 on success, -EIO on SPI communication failure
 */
int pat9130ew::delta_read(int16_t &delta_x, int16_t &delta_y) {
    int res;

    /* Read registers */
    uint8_t delta_x_low = 0x00;
    uint8_t delta_y_low = 0x00;
    uint8_t delta_x_high = 0x00;
    uint8_t delta_y_high = 0x00;
    int16_t delta_x_value = 0;
    int16_t delta_y_value = 0;
    res = register_read(PAT9130EW_REGISTER_DELTA_X_L, delta_x_low);
    if (res < 0) {
        return -EIO;
    }
    res = register_read(PAT9130EW_REGISTER_DELTA_Y_L, delta_y_low);
    if (res < 0) {
        return -EIO;
    }
    res = register_read(PAT9130EW_REGISTER_DELTA_X_H, delta_x_high);
    if (res < 0) {
        return -EIO;
    }
    res = register_read(PAT9130EW_REGISTER_DELTA_Y_H, delta_y_high);
    if (res < 0) {
        return -EIO;
    }

    /* Convert registers to signed 16-bit values */
    delta_x_value = ((int16_t)(delta_x_high << 8)) & 0xFF00;
    if (delta_x_value & 0x8000) {
        delta_x_value |= 0xF000;
    }
    delta_x_value |= (int16_t)delta_x_low;
    delta_y_value = ((int16_t)(delta_y_high << 8)) & 0xFF00;
    if (delta_y_value & 0x8000) {
        delta_y_value |= 0xF000;
    }
    delta_y_value |= (int16_t)delta_y_low;

    /* Return values */
    delta_x = delta_x_value;
    delta_y = delta_y_value;

    /* Return success */
    return 0;
}
