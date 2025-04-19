#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "lcd.h"

//*************************************************************************************************

#define IR_BEAM_INPUT_GPIO_PIN 0
#define COUNTER_RESET_GPIO_PIN 1

int counter = 0;
int64_t breakTimeDiff = 0;

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
                breakTimeDiff = diff;
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

    gpio_init(COUNTER_RESET_GPIO_PIN);
    gpio_set_dir(COUNTER_RESET_GPIO_PIN, GPIO_IN);
    gpio_pull_up(COUNTER_RESET_GPIO_PIN);
    gpio_set_irq_enabled(COUNTER_RESET_GPIO_PIN, GPIO_IRQ_EDGE_FALL, true);
    lcd_init();

    char buffer[20];
    int prevCounter = -1;
    int64_t prevBreakTimeDiff = -1;

    while (1)
    {
        if ((counter != prevCounter) || (breakTimeDiff != prevBreakTimeDiff))
        {
            lcd_clear();

            lcd_set_cursor(0, 0);
            snprintf(buffer, sizeof(buffer), "%d detection(s)", counter);
            lcd_set_string(buffer);
            printf("Counter is %s\n", buffer);

            lcd_set_cursor(1, 0);
            snprintf(buffer, sizeof(buffer), "%lli us", breakTimeDiff);
            lcd_set_string(buffer);
            printf("Time diff is %s\n", buffer);

            // TDOO: remove
            //printf("currentTime = %llu\n", currentTime);

            prevCounter = counter;
            prevBreakTimeDiff = breakTimeDiff;
        }

        sleep_ms(100);
    }
}
