#include "pwmBuzzer.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

//*************************************************************************************************

#define PWM_BUZZER_PIN 15
static uint slice_num = 0;

//*************************************************************************************************

void pwm_buzzer_init()
{
    gpio_set_function(PWM_BUZZER_PIN, GPIO_FUNC_PWM); // set the GPIO to PWM
    slice_num = pwm_gpio_to_slice_num(PWM_BUZZER_PIN); // Find out which PWM slice is connected to the GPIO
}

//*************************************************************************************************

void pwm_buzzer_on()
{
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125); // ex: 125MHz / 125 = 1MHz counter
    pwm_config_set_wrap(&config, 1000); // set period of PWM
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(PWM_BUZZER_PIN, 500); // set number of cycles it will be high before dropping
}

//*************************************************************************************************

void pwm_buzzer_off()
{
    pwm_set_gpio_level(PWM_BUZZER_PIN, 0); // set number of cycles it will be high before dropping
    pwm_set_enabled(slice_num, false);
}
