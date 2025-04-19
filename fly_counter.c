#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "lcd.h"

//*************************************************************************************************

#define IR_BEAM_INPUT_GPIO_PIN 0
#define COUNTER_RESET_GPIO_PIN 1

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

    lcd_init();

    char buffer[20];
    int prevCounter = -1;
    int64_t prevBreakTime = -1;

    while (1)
    {
        if (counter != prevCounter)
        {
            lcd_clear();

            lcd_set_cursor(0, 0);
            snprintf(buffer, sizeof(buffer), "%d detection(s)", counter);
            lcd_set_string(buffer);
            printf("%s\n", buffer);

            prevCounter = counter;

            // TODO: turn on buzzer
        }

        if (breakTimeMicroSecs != prevBreakTime)
        {
            lcd_set_cursor(1, 0);
            uint64_t ms = breakTimeMicroSecs / 1000;
            uint64_t us = breakTimeMicroSecs % 1000;
            snprintf(buffer, sizeof(buffer), "%llu.%03llu ms", ms, us);
            lcd_set_string(buffer);
            printf("%s\n", buffer);
            
            // TODO: remove all USB comms?
            snprintf(buffer, sizeof(buffer), "%lli us", breakTimeMicroSecs);
            printf("Time diff is %s\n", buffer);

            prevBreakTime = breakTimeMicroSecs;
        }

        sleep_ms(100);
    }
}
