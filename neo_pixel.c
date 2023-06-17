#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "pico/rand.h"


// PWM: 125(SYSCLK) / 12.5 = 10MHz(0.1us)
#define NEOPIXEL_PWM_DIVCLK 12.5

/*
RP2040 XIAO RGB LED
https://www.mouser.jp/datasheet/2/813/IN-PI22TAT5R5G5B_v1.1-1509120.pdf
Data transfer time(TH+TL=1.2us)
- T0H: 0.3 us
- T0L: 0.9 us
- T1H: 0.9 us
- T1L: 0.3 us
- RES: 80  us
*/
#define NEOPIXEL_PWM_WRAP 12
#define NEOPIXEL_PWM_T0H 3
#define NEOPIXEL_PWM_T1H 9

// RGB888
#define NEOPIXEL_DATA_BITS 24

// RP2040 XIAO RGB LED Pin assignment
#define NEOPIXEL_POWER_PIN 11
#define NEOPIXEL_PWM_PIN 12

int main() {
    // Power on NEOPIXEL
    gpio_init(NEOPIXEL_POWER_PIN);
    gpio_set_dir(NEOPIXEL_POWER_PIN, GPIO_OUT);
    gpio_put(NEOPIXEL_POWER_PIN, true);

    // PWM setings
    gpio_set_function(NEOPIXEL_PWM_PIN, GPIO_FUNC_PWM);
    uint pwm_slice_num = pwm_gpio_to_slice_num(NEOPIXEL_PWM_PIN);
    pwm_set_wrap(pwm_slice_num, NEOPIXEL_PWM_WRAP);
    pwm_set_clkdiv(pwm_slice_num, NEOPIXEL_PWM_DIVCLK);
    pwm_set_irq_enabled(pwm_slice_num, true);
    pwm_set_enabled(pwm_slice_num, true);

    // DMA settings
    uint dma_channel = dma_claim_unused_channel(true);
    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_dreq(&dma_config, pwm_get_dreq(pwm_slice_num));
    dma_channel_configure(
        dma_channel,
        &dma_config,
        &pwm_hw->slice[pwm_slice_num].cc,
        NULL,
        NEOPIXEL_DATA_BITS + 1,
        false
    );

    // Random lighting loop
    while(true) {
        uint32_t data_24bits = rand();
        uint32_t pwm_counter_compare[NEOPIXEL_DATA_BITS + 4] = {0};
        for (uint32_t i = 0; i < NEOPIXEL_DATA_BITS; i++) {
            if (data_24bits & (1 << i)) {
                pwm_counter_compare[i] = NEOPIXEL_PWM_T1H;
            } else {
                pwm_counter_compare[i] = NEOPIXEL_PWM_T0H;
            }
        }
        dma_channel_set_read_addr(dma_channel, &pwm_counter_compare, false);
        dma_channel_start(dma_channel);
        busy_wait_ms(500);
    }
}
