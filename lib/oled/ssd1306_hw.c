#include "ssd1306_hw.h"
#include "pin_config.h"

static I2C_HandleTypeDef *oled_hi2c;

void ssd1306_hw_bind(I2C_HandleTypeDef *hi2c)
{
    oled_hi2c = hi2c;
}

static void hw_write_cmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(oled_hi2c, (uint16_t)(OLED_I2C_ADDRESS << 1), buf, 2, 100);
}

static void hw_write_data(const uint8_t *data, uint16_t len)
{
    uint8_t buf[17];
    uint16_t offset = 0;

    while (offset < len) {
        uint16_t chunk = (len - offset > 16U) ? 16U : (len - offset);
        buf[0] = 0x40;
        for (uint16_t i = 0; i < chunk; i++) {
            buf[i + 1U] = data[offset + i];
        }
        HAL_I2C_Master_Transmit(oled_hi2c, (uint16_t)(OLED_I2C_ADDRESS << 1),
                                buf, (uint16_t)(chunk + 1U), 100);
        offset += chunk;
    }
}

void ssd1306_hw_init(void)
{
    HAL_Delay(100);

    hw_write_cmd(0xAE);
    hw_write_cmd(0xD5);
    hw_write_cmd(0x80);
    hw_write_cmd(0xA8);
    hw_write_cmd(0x3F);
    hw_write_cmd(0xD3);
    hw_write_cmd(0x00);
    hw_write_cmd(0x40);
    hw_write_cmd(0x8D);
    hw_write_cmd(0x14);
    hw_write_cmd(0x20);
    hw_write_cmd(0x00);
    hw_write_cmd(0xA1);
    hw_write_cmd(0xC8);
    hw_write_cmd(0xDA);
    hw_write_cmd(0x12);
    hw_write_cmd(0x81);
    hw_write_cmd(0xCF);
    hw_write_cmd(0xD9);
    hw_write_cmd(0xF1);
    hw_write_cmd(0xDB);
    hw_write_cmd(0x40);
    hw_write_cmd(0xA4);
    hw_write_cmd(0xA6);
    hw_write_cmd(0xAF);
}

void ssd1306_hw_flush(const uint8_t *framebuffer)
{
    hw_write_cmd(0x21);
    hw_write_cmd(0x00);
    hw_write_cmd(0x7F);
    hw_write_cmd(0x22);
    hw_write_cmd(0x00);
    hw_write_cmd(0x07);
    hw_write_data(framebuffer, 1024);
}
