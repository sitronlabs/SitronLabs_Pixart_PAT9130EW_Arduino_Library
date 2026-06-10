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
    PAT9130EW_REGISTER_WRITE_PROTECT = 0x09,
    PAT9130EW_REGISTER_RES_X = 0x0D,
    PAT9130EW_REGISTER_RES_Y = 0x0E,
    PAT9130EW_REGISTER_DELTA_X_H = 0x11,
    PAT9130EW_REGISTER_DELTA_Y_H = 0x12,
    PAT9130EW_REGISTER_IQ = 0x13,
    PAT9130EW_REGISTER_SHUTTER = 0x15,
    PAT9130EW_REGISTER_FRAME_AVG = 0x17,
    PAT9130EW_REGISTER_MFIO_CONFIG = 0x26,
    PAT9130EW_REGISTER_BANK_SELECT = 0x7F,
};

/**
 * MFIO pin function (datasheet section 8.2.15, MFIO_Config[2:0] at bits 5:3)
 */
enum pat9130ew_mfio_function {
    PAT9130EW_MFIO_HARDWARE_RESET = 0,
    PAT9130EW_MFIO_QUICK_BURST = 3,
    PAT9130EW_MFIO_HARDWARE_POWER_DOWN = 4,
    PAT9130EW_MFIO_NULL = 6,
};

/**
 * PAT9130EW motion tracking sensor driver class
 *
 * This class provides an interface to communicate with the PixArt PAT9130EW laser-based optical motion tracking sensor over 3-wire SPI. The SDIO data pin on the sensor must be connected to both the MOSI and MISO pins of the host.
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
     * Check whether new motion delta data is available
     * @return 1 if the motion bit is set, 0 if not, negative error code on failure
     */
    int motion_delta_available(void);

    /**
     * Read accumulated X/Y motion deltas when available
     *
     * Reads Motion_Status first (required by the datasheet), then delta registers when the motion bit is set.
     *
     * @param[out] delta_x X displacement in counts (valid only when return value is 1)
     * @param[out] delta_y Y displacement in counts (valid only when return value is 1)
     * @param[out] x_overflow true if the X motion buffer overflowed since the last read
     * @param[out] y_overflow true if the Y motion buffer overflowed since the last read
     * @return 1 if motion data was read, 0 if no motion data is available, negative error code on failure
     */
    int motion_delta_read(int16_t &delta_x, int16_t &delta_y, bool &x_overflow, bool &y_overflow);

    /**
     * Read image quality (datasheet register IQ, address 0x13)
     * @param[out] iq Image quality value (0 to 255)
     * @return 0 on success, negative error code on failure
     */
    int iq_read(uint8_t &iq);

    /**
     * Read shutter index (datasheet register Shutter, address 0x15)
     * @param[out] shutter Shutter value (0 to 255)
     * @return 0 on success, negative error code on failure
     */
    int shutter_read(uint8_t &shutter);

    /**
     * Read average frame brightness (datasheet register FrameAvg, address 0x17)
     * @param[out] frame_avg Average brightness (0 to 255)
     * @return 0 on success, negative error code on failure
     */
    int frame_avg_read(uint8_t &frame_avg);

    /**
     * Read motion, surface quality, and delta data in one SPI burst transaction (datasheet section 7.4.4)
     * @param[out] delta_x X displacement in counts
     * @param[out] delta_y Y displacement in counts
     * @param[out] motion_status Raw Motion_Status register value (bit 7 = motion available)
     * @param[out] x_overflow true if the X motion buffer overflowed since the last read
     * @param[out] y_overflow true if the Y motion buffer overflowed since the last read
     * @param[out] iq Image quality value (0 to 255)
     * @param[out] shutter Shutter value (0 to 255)
     * @param[out] frame_avg Average frame brightness (0 to 255)
     * @return 0 on success, negative error code on failure
     */
    int burst_read(int16_t &delta_x, int16_t &delta_y, uint8_t &motion_status, bool &x_overflow, bool &y_overflow, uint8_t &iq, uint8_t &shutter, uint8_t &frame_avg);

    /**
     * Read the configured CPI resolution for both axes
     * @param[out] cpi_x Estimated CPI for the X axis (55.2 x RES_X)
     * @param[out] cpi_y Estimated CPI for the Y axis (55.2 x RES_Y)
     * @return 0 on success, negative error code on failure
     */
    int resolution_cpi_get(float &cpi_x, float &cpi_y);

    /**
     * Enter or exit software power-down mode (Configuration register PD_EnH, bit 3)
     * @param[in] enable true to enter power-down, false to resume normal operation
     * @return 0 on success, negative error code on failure
     */
    int power_down(const bool enable);

    /**
     * Disable or enable register write protection (Write_Protect register, value 0x5A)
     * @param[in] disable true to disable write protection, false to enable write protection
     * @return 0 on success, negative error code on failure
     */
    int write_protect_disable(const bool disable);

    /**
     * Configure the MFIO pin function (MFIO_Config register, bits 5:3)
     * @param[in] function MFIO pin function to select
     * @return 0 on success, negative error code on failure
     */
    int mfio_config_set(const enum pat9130ew_mfio_function function);

   protected:
    SPIClass *m_spi_library = NULL;
    int m_pin_cs = -1;
    uint32_t m_spi_speed = 1000000;
};

#endif
