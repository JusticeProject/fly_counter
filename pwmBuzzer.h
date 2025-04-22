#ifndef _PWM_BUZZER_H_
#define _PWM_BUZZER_H_

#include <stdint.h>

//*************************************************************************************************

void pwm_buzzer_init();
void pwm_buzzer_on();
void pwm_buzzer_off();
int64_t pwm_on_time_ms();

#endif
