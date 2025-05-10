#include "pwmBuzzer.h"
#include "hardware/pwm.h"

//*************************************************************************************************

static uint buzzer_gpio_pin = 0;
static uint slice_num = 0;

static bool pwm_is_on = false;
static absolute_time_t pwm_turn_on_time = 0;

//*************************************************************************************************

void pwm_buzzer_init(uint pin)
{
    buzzer_gpio_pin = pin;
    gpio_set_function(buzzer_gpio_pin, GPIO_FUNC_PWM); // set the GPIO to PWM
    slice_num = pwm_gpio_to_slice_num(buzzer_gpio_pin); // Find out which PWM slice is connected to the GPIO
}

//*************************************************************************************************

void pwm_buzzer_on()
{
    if (pwm_is_on)
    {
        return;
    }

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125); // ex: 125MHz / 125 = 1MHz counter
    pwm_config_set_wrap(&config, 1000); // set period of PWM
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(buzzer_gpio_pin, 500); // set number of cycles it will be high before dropping
    
    pwm_turn_on_time = get_absolute_time();
    pwm_is_on = true;
}

//*************************************************************************************************

void pwm_buzzer_off()
{
    pwm_set_gpio_level(buzzer_gpio_pin, 0); // set number of cycles it will be high before dropping
    pwm_set_enabled(slice_num, false);

    pwm_is_on = false;
}

//*************************************************************************************************

int64_t pwm_on_time_ms()
{
    if (!pwm_is_on)
    {
        return 0;
    }

    absolute_time_t now = get_absolute_time();
    int64_t diff = absolute_time_diff_us(pwm_turn_on_time, now);
    return diff / 1000;
}
