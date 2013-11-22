/**
 * @file    max6675.c
 *
 * @author  David Zemon
 */

#include <max6675.h>

uint32_t g_max6675_cs = -1;
uint8_t g_max6675_alwaysSetMode = 0;

int8_t MAX6675Start (const uint32_t mosi, const uint32_t miso,
        const uint32_t sclk, const uint32_t cs) {
    int8_t err;

    if (!SPIIsRunning()) {
        checkErrors(
                SPIStart(mosi, miso, sclk, MAX6675_SPI_DEFAULT_FREQ, MAX6675_SPI_MODE, MAX6675_SPI_BITMODE));
    } else {
        checkErrors(SPISetMode(MAX6675_SPI_MODE));
        checkErrors(SPISetBitMode(MAX6675_SPI_BITMODE));
    }

    g_max6675_cs = cs;
    GPIODirModeSet(cs, GPIO_DIR_OUT);

    return 0;
}

void MAX6675AlwaysSetMode (const uint8_t alwaysSetMode) {
    g_max6675_alwaysSetMode = alwaysSetMode;
}

int8_t MAX6675Read (uint16_t *dat) {
    int8_t err;

    if (g_max6675_alwaysSetMode) {
        checkErrors(SPISetMode(MAX6675_SPI_MODE));
        checkErrors(SPISetBitMode(MAX6675_SPI_BITMODE));
    }

    *dat = 0;
    GPIOPinClear(g_max6675_cs);
    checkErrors(SPIShiftIn(MAX6675_BIT_WIDTH, dat, sizeof(*dat)));
    GPIOPinSet(g_max6675_cs);

    return 0;
}

int8_t MAX6675ReadWhole (uint16_t *dat) {
    int8_t err;

    checkErrors(MAX6675Read(dat));
    *dat >>= 2;

    return 0;
}

int8_t MAX6675ReadFloat (float *dat) {
    int8_t err;
    uint16_t temp;

    checkErrors(MAX6675Read(&temp));

    *dat = temp >> 2;
    *dat += ((float) (temp & (BIT_1 | BIT_0))) / 4;

    return 0;
}
