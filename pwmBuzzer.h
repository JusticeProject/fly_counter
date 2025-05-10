#ifndef _PWM_BUZZER_H_
#define _PWM_BUZZER_H_

#include <stdint.h>
#include "pico/stdlib.h"

//*************************************************************************************************

void pwm_buzzer_init(uint pin);
void pwm_buzzer_on();
void pwm_buzzer_off();
int64_t pwm_on_time_ms();

#endif
