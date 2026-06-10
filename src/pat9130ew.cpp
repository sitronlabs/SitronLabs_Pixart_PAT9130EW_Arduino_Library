/* Self header */
#include "pat9130ew.h"

/**
 * Default register settings from PixArt PAT9130EW reference firmware.
 *
 * Derived from the "initialize" table in registerArrays.h (9130_referenceCode),
 * Apache License 2.0. See README.md for source links.
 */
static const uint8_t m_init_sequence[][2] = {
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
static const size_t m_init_sequence_length = sizeof(m_init_sequence) / sizeof(m_init_sequence[0]);

/**
 * Decode the delta values from the motion status register
 * @param[in] delta_low The low byte of the delta value.
 * @param[in] delta_high The high byte of the delta value.
 * @return The decoded delta value.
 */
static int16_t m_decode_delta(const uint8_t delta_low, const uint8_t delta_high) {
    int16_t delta_value = ((int16_t)(delta_high << 8)) & 0xFF00;
    if (delta_value & 0x8000) {
        delta_value |= 0xF000;
    }
    delta_value |= (int16_t)delta_low;
    return delta_value;
}

/**
 * Parse the motion status register
 * @param[in] motion_status The motion status register value.
 * @param[out] x_overflow True if the X overflow bit is set, false otherwise.
 * @param[out] y_overflow True if the Y overflow bit is set, false otherwise.
 */
static void m_parse_motion_status(const uint8_t motion_status, bool &x_overflow, bool &y_overflow) {
    x_overflow = (motion_status & 0x10) != 0;
    y_overflow = (motion_status & 0x20) != 0;
}

/**
 * Initialize the PAT9130EW sensor with SPI communication parameters
 *
 * Stores the SPI instance, chip select pin, and clock speed for subsequent operations. The SDIO pin on the sensor must be wired to both MOSI and MISO.
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

    /* Begin transaction */
    m_spi_library->beginTransaction(SPISettings(m_spi_speed, MSBFIRST, SPI_MODE3));
    digitalWrite(m_pin_cs, LOW);

    /* Write register address */
    uint8_t address;
    address = ((uint8_t)reg_address) & 0x7F;
    m_spi_library->transfer(address);

    /* Read register content */
    delayMicroseconds(1);
    reg_content = m_spi_library->transfer(0x00);

    /* End transaction */
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
    /* Begin transaction */
    m_spi_library->beginTransaction(SPISettings(m_spi_speed, MSBFIRST, SPI_MODE3));
    digitalWrite(m_pin_cs, LOW);

    /* Write register address and content */
    address = ((uint8_t)reg_address) | 0x80;
    m_spi_library->transfer(address);
    m_spi_library->transfer(reg_content);

    /* End transaction */
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
 * @return 0 if a valid PAT9130EW device is detected, -EIO on SPI communication failure, or -1 if the product ID does not match
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
 * Issues a software reset, waits for the device to recover, then loads the default register settings from the PixArt reference firmware.
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
    for (size_t index = 0; index < m_init_sequence_length; index++) {
        res = register_write((enum pat9130ew_register)m_init_sequence[index][0], m_init_sequence[index][1]);
        if (res < 0) {
            return -EIO;
        }
    }

    /* Return success */
    return 0;
}

/**
 * Check whether new motion delta data is available
 * @return 1 if the motion bit is set, 0 if not, negative error code on failure
 */
int pat9130ew::motion_delta_available(void) {
    int res;

    /* Read motion status */
    uint8_t motion_status = 0x00;
    res = register_read(PAT9130EW_REGISTER_MOTION, motion_status);
    if (res < 0) {
        return -EIO;
    }

    /* Return motion data availability */
    return ((motion_status & 0x80) != 0) ? 1 : 0;
}

/**
 * Read accumulated X/Y motion deltas when available
 * @return 1 if motion data was read, 0 if no motion data is available, negative error code on failure
 */
int pat9130ew::motion_delta_read(int16_t &delta_x, int16_t &delta_y, bool &x_overflow, bool &y_overflow) {
    int res;

    /* Read motion status and return now if no motion data is available */
    uint8_t motion_status = 0x00;
    res = register_read(PAT9130EW_REGISTER_MOTION, motion_status);
    if (res < 0) {
        return -EIO;
    }
    m_parse_motion_status(motion_status, x_overflow, y_overflow);
    if ((motion_status & 0x80) == 0) {
        return 0;
    }

    /* Read delta registers */
    uint8_t delta_x_low = 0x00;
    uint8_t delta_y_low = 0x00;
    uint8_t delta_x_high = 0x00;
    uint8_t delta_y_high = 0x00;
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
    delta_x = m_decode_delta(delta_x_low, delta_x_high);
    delta_y = m_decode_delta(delta_y_low, delta_y_high);

    /* Return success */
    return 1;
}

/**
 * Read image quality (register IQ, address 0x13)
 * @param[out] iq Image quality value (0 to 255)
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::iq_read(uint8_t &iq) {
    return register_read(PAT9130EW_REGISTER_IQ, iq);
}

/**
 * Read shutter index (register Shutter, address 0x15)
 * @param[out] shutter Shutter value (0 to 255)
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::shutter_read(uint8_t &shutter) {
    return register_read(PAT9130EW_REGISTER_SHUTTER, shutter);
}

/**
 * Read average frame brightness (register FrameAvg, address 0x17)
 * @param[out] frame_avg Average brightness (0 to 255)
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::frame_avg_read(uint8_t &frame_avg) {
    return register_read(PAT9130EW_REGISTER_FRAME_AVG, frame_avg);
}

/**
 * Read motion, surface quality, and delta data in one SPI burst transaction
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::burst_read(int16_t &delta_x, int16_t &delta_y, uint8_t &motion_status, bool &x_overflow, bool &y_overflow, uint8_t &iq, uint8_t &shutter, uint8_t &frame_avg) {
    uint8_t burst_data[11];

    /* Ensure library has been configured */
    if ((m_spi_library == NULL) || (m_pin_cs < 0)) {
        return -EINVAL;
    }

    /* Start transaction */
    m_spi_library->beginTransaction(SPISettings(m_spi_speed, MSBFIRST, SPI_MODE3));
    digitalWrite(m_pin_cs, LOW);
    delayMicroseconds(1);

    /* Start burst read*/
    m_spi_library->transfer(0x82);

    /* Read burst data */
    for (size_t index = 0; index < sizeof(burst_data); index++) {
        burst_data[index] = m_spi_library->transfer(0x00);
    }

    /* End transaction */
    digitalWrite(m_pin_cs, HIGH);
    m_spi_library->endTransaction();

    /* Parse burst data */
    motion_status = burst_data[0];
    delta_x = m_decode_delta(burst_data[1], burst_data[3]);
    delta_y = m_decode_delta(burst_data[2], burst_data[4]);
    iq = burst_data[5];
    shutter = burst_data[7];
    frame_avg = burst_data[9];
    m_parse_motion_status(motion_status, x_overflow, y_overflow);

    /* Return success */
    return 0;
}

/**
 * Read the configured CPI resolution for both axes
 * @param[out] cpi_x Estimated CPI for the X axis (55.2 x RES_X)
 * @param[out] cpi_y Estimated CPI for the Y axis (55.2 x RES_Y)
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::resolution_cpi_get(float &cpi_x, float &cpi_y) {
    int res;

    /* Read resolution registers */
    uint8_t res_x = 0x00;
    uint8_t res_y = 0x00;
    res = register_read(PAT9130EW_REGISTER_RES_X, res_x);
    if (res < 0) {
        return res;
    }
    res = register_read(PAT9130EW_REGISTER_RES_Y, res_y);
    if (res < 0) {
        return res;
    }

    /* Convert resolution registers to CPI */
    static const float k_cpi_per_step = 55.2f;
    cpi_x = k_cpi_per_step * (float)(res_x & 0x7F);
    cpi_y = k_cpi_per_step * (float)(res_y & 0x7F);

    /* Return success */
    return 0;
}

/**
 * Enter or exit software power-down mode
 * @param[in] enable true to enter power-down, false to resume normal operation
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::power_down(const bool enable) {
    int res;

    /* Read configuration register */
    uint8_t configuration = 0x00;
    res = register_read(PAT9130EW_REGISTER_CONFIGURATION, configuration);
    if (res < 0) {
        return res;
    }

    /* Modify power-down bit */
    if (enable) {
        configuration |= 0x08;
    } else {
        configuration &= (uint8_t)~0x08;
    }
    res = register_write(PAT9130EW_REGISTER_CONFIGURATION, configuration);
    if (res < 0) {
        return res;
    }

    /* Return success */
    return 0;
}

/**
 * Disable or enable register write protection
 * @param[in] disable true to disable write protection, false to enable write protection
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::write_protect_disable(const bool disable) {
    uint8_t reg = disable ? 0x5A : 0x00;
    return register_write(PAT9130EW_REGISTER_WRITE_PROTECT, reg);
}

/**
 * Configure the MFIO pin function
 * @param[in] function MFIO pin function to select
 * @return 0 on success, negative error code on failure
 */
int pat9130ew::mfio_config_set(const enum pat9130ew_mfio_function function) {
    int res;
    uint8_t mfio_config = 0x00;

    res = write_protect_disable(true);
    if (res < 0) {
        return res;
    }

    res = register_read(PAT9130EW_REGISTER_MFIO_CONFIG, mfio_config);
    if (res < 0) {
        write_protect_disable(false);
        return res;
    }

    mfio_config = (uint8_t)((mfio_config & (uint8_t)~0x38) | (((uint8_t)function & 0x07) << 3));

    res = register_write(PAT9130EW_REGISTER_MFIO_CONFIG, mfio_config);
    if (res < 0) {
        write_protect_disable(false);
        return res;
    }

    return write_protect_disable(false);
}
