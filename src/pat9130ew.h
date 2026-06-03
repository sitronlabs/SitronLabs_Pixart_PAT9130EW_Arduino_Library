#ifndef PAT9130EW_H
#define PAT9130EW_H

/* Arduino libraries */
#include <Arduino.h>
#include <SPI.h>

/* C/C++ libraries */
#include <errno.h>
#include <stdint.h>

/**
 * PAT9130EW register addresses (bank 0 unless written via register 0x7F)
 */
enum pat9130ew_register {
    PAT9130EW_REGISTER_PRODUCT_ID = 0x00,
    PAT9130EW_REGISTER_REVISION_ID = 0x01,
    PAT9130EW_REGISTER_MOTION = 0x02,
    PAT9130EW_REGISTER_DELTA_X_L = 0x03,
    PAT9130EW_REGISTER_DELTA_Y_L = 0x04,
    PAT9130EW_REGISTER_OPERATION_MODE = 0x05,
    PAT9130EW_REGISTER_CONFIGURATION = 0x06,
    PAT9130EW_REGISTER_DELTA_X_H = 0x11,
    PAT9130EW_REGISTER_DELTA_Y_H = 0x12,
    PAT9130EW_REGISTER_BANK_SELECT = 0x7F,
};

/**
 * PAT9130EW motion tracking sensor driver class
 *
 * This class provides an interface to communicate with the PixArt PAT9130EW
 * laser-based optical motion tracking sensor over 3-wire SPI. The SDIO data pin
 * on the sensor must be connected to both the MOSI and MISO pins of the host.
 */
class pat9130ew {
   public:
    /**
     * Initialize the PAT9130EW sensor with SPI communication parameters
     * @param[in] spi_library Reference to the SPIClass instance to use
     * @param[in] pin_cs GPIO pin connected to the chip select (active low)
     * @param[in] spi_speed SPI clock speed in Hz (default 1 MHz per reference firmware)
     * @return 0 on success, negative error code on failure
     */
    int setup(SPIClass &spi_library, const int pin_cs, const uint32_t spi_speed = 1000000);

    /**
     * Detect and verify the presence of a PAT9130EW sensor
     * Reads and validates the product ID (0x31)
     * @return 0 if a valid PAT9130EW device is detected, negative error code otherwise
     */
    int detect(void);

    /**
     * Initialize the sensor (software reset and default register settings)
     * @return 0 on success, negative error code on failure
     */
    int init(void);

    /**
     * Read an 8-bit register from the PAT9130EW sensor
     * @param[in] reg_address Register address to read from
     * @param[out] reg_content Variable that will receive the register value
     * @return 0 on success, negative error code on failure (-EINVAL if not initialized, -EIO on SPI error)
     */
    int register_read(const enum pat9130ew_register reg_address, uint8_t &reg_content);

    /**
     * Write an 8-bit value to a PAT9130EW register
     * @param[in] reg_address Register address to write to
     * @param[in] reg_content 8-bit value to write to the register
     * @return 0 on success, negative error code on failure (-EINVAL if not initialized, -EIO on SPI error)
     */
    int register_write(const enum pat9130ew_register reg_address, const uint8_t reg_content);

    /**
     * Check whether new motion data is available
     * @return true if the motion bit is set in the motion status register, false otherwise
     */
    bool motion_available(void);

    /**
     * Read the latest X/Y displacement since the previous report
     * @param[out] delta_x Variable that will receive the X displacement in counts
     * @param[out] delta_y Variable that will receive the Y displacement in counts
     * @return 0 on success, negative error code on SPI communication failure
     */
    int delta_read(int16_t &delta_x, int16_t &delta_y);

   protected:
    SPIClass *m_spi_library = NULL;
    int m_pin_cs = -1;
    uint32_t m_spi_speed = 1000000;
};

#endif
