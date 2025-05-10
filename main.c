#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "lcd.h"
#include "pwmBuzzer.h"
#include "ultrasonic_distance.h"

//*************************************************************************************************

#define IR_BEAM_INPUT_GPIO_PIN 0
#define COUNTER_RESET_GPIO_PIN 1
#define I2C_PORT i2c0
#define I2C_SDA_GPIO_PIN 4
#define I2C_SCL_GPIO_PIN 5
#define PWM_BUZZER_GPIO_PIN 15

int counter = 0;
int64_t breakTimeMicroSecs = 0;

//*************************************************************************************************

void gpio_callback(uint gpio, uint32_t events)
{
    static absolute_time_t breakTime = 0;
    static absolute_time_t unbreakTime = 0;
    
    absolute_time_t currentTime = get_absolute_time();

    if (IR_BEAM_INPUT_GPIO_PIN == gpio)
    {
        if (events & GPIO_IRQ_EDGE_FALL)
        {
            counter++;
            breakTime = currentTime;
        }

        if (events & GPIO_IRQ_EDGE_RISE)
        {
            unbreakTime = currentTime;

            int64_t diff = absolute_time_diff_us(breakTime, unbreakTime);
            if (diff > 0)
            {
                breakTimeMicroSecs = diff;
            }
        }
    }
    else if (COUNTER_RESET_GPIO_PIN == gpio)
    {
        if (events & GPIO_IRQ_EDGE_FALL)
        {
            counter = 0;
            breakTimeMicroSecs = 0;
        }
    }
}

//*************************************************************************************************

int main()
{
    stdio_init_all();
    
    gpio_init(IR_BEAM_INPUT_GPIO_PIN);
    gpio_set_dir(IR_BEAM_INPUT_GPIO_PIN, GPIO_IN);
    gpio_pull_up(IR_BEAM_INPUT_GPIO_PIN);
    gpio_set_irq_enabled_with_callback(IR_BEAM_INPUT_GPIO_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    // the first GPIO irq is registered with the callback, the next one we register doesn't need to specify the callback since it's the same

    gpio_init(COUNTER_RESET_GPIO_PIN);
    gpio_set_dir(COUNTER_RESET_GPIO_PIN, GPIO_IN);
    gpio_pull_up(COUNTER_RESET_GPIO_PIN);
    gpio_set_irq_enabled(COUNTER_RESET_GPIO_PIN, GPIO_IRQ_EDGE_FALL, true);

    pwm_buzzer_init(PWM_BUZZER_GPIO_PIN);
    lcd_init(I2C_PORT, I2C_SDA_GPIO_PIN, I2C_SCL_GPIO_PIN);
    lcd_clear();
    ultrasonic_distance_init();

    char buffer[40]; // overkill, but the large size is useful when printing debug messages
    int prevCounter = -1;
    int64_t prevBreakTime = -1;

    while (1)
    {
        if (counter != prevCounter)
        {
            // we will use prevCounter for the rest of this block because counter could change again while we analyze it
            prevCounter = counter;

            lcd_set_cursor(0, 0);
            snprintf(buffer, sizeof(buffer), "%d detection(s)  ", prevCounter);
            lcd_set_string(buffer);
            //printf("%s\n", buffer);

            if (prevCounter > 0)
            {
                pwm_buzzer_on();
            }
        }

        if (breakTimeMicroSecs != prevBreakTime)
        {
            // We will use prevBreakTime for the rest of this block because breakTimeMicroSecs could change again while we analyze it.
            // It could have changed between the time we tested it and the time we saved it, but for this project it's not a big deal.
            // If we really cared about the value we could have the ISR put the value in a queue that the main thread would grab 
            // (or turn off interrupts temporarily).
            prevBreakTime = breakTimeMicroSecs;

            lcd_set_cursor(1, 0);
            uint64_t ms = prevBreakTime / 1000;
            uint64_t us = prevBreakTime % 1000;
            snprintf(buffer, sizeof(buffer), "%llu.%03llu ms        ", ms, us);
            lcd_set_string(buffer);
            //printf("%s\n", buffer);
            
            //snprintf(buffer, sizeof(buffer), "%lli us", prevBreakTime);
            //printf("Time diff is %s\n", buffer);
        }

        int64_t on_time = pwm_on_time_ms();
        if (on_time > 1000)
        {
            pwm_buzzer_off();
        }

        sleep_ms(100);
    }
}
