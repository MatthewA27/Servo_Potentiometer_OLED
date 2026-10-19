#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

#include "ssd1306.h"

#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15

#define SERVO_PIN 5
#define POT_PIN 26

int main()
{
    stdio_init_all();

    // ADC Setup
    adc_init();
    adc_gpio_init(POT_PIN);
    adc_select_input(0);

    // PWM Setup
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    // 125 MHz / 125 = 1 MHz PWM clock
    pwm_set_clkdiv(slice_num, 125.0);

    // 1 MHz clock means 1 count = 1 µs
    // 20,000 counts = 20 ms = 50 Hz
    pwm_set_wrap(slice_num, 20000);

    i2c_init(I2C_PORT, 400000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_t disp;

    disp.external_vcc = false;

    ssd1306_init(
        &disp,
        128,
        64,
        0x3C,
        I2C_PORT
    );

    pwm_set_enabled(slice_num, true);

    while (true)
    {
        int adc_value = adc_read();

        int angle = adc_value * 180 / 4095;

        // Map 0-180° to 1000-2000 µs
        int pulse_width = 500 + (angle * 2000 / 180);

        pwm_set_gpio_level(SERVO_PIN, pulse_width);

        printf("ADC: %d  Angle: %d\n",
               adc_value,
               angle);
        
        ssd1306_clear(&disp);

        char line1[32];
        char line2[32];

        sprintf(line1, "ADC: %d", adc_value);
        sprintf(line2, "Angle: %d", angle);

        ssd1306_draw_string(
            &disp,
            0,
            0,
            2,
            line1
        );

        ssd1306_draw_string(
            &disp,
            0,
            20,
            2,
            line2
        );

        ssd1306_show(&disp);

        sleep_ms(20);
    }
}