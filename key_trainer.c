#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "play_audio.h"

// ---------------------------------------------------------------------------
// Pin and ADC definitions
// ---------------------------------------------------------------------------
#define LED_PIN_RED     13
#define LED_PIN_GREEN   11
#define BUTTON_PIN1     5
#define BUTTON_PIN2     6

const uint I2C_SDA_PIN = 14;
const uint I2C_SCL_PIN = 15;

#define ADC_INPUT_PIN   28
#define ADC_CHANNEL     2

// ---------------------------------------------------------------------------
// ADC Sampling parameters for note detection
// ---------------------------------------------------------------------------
#define SAMPLE_RATE     2000.0f             // samples per second
#define NUM_SAMPLES     200                 // number of samples per detection cycle
#define SAMPLE_DELAY_US (1000000 / SAMPLE_RATE)
#define NOISE_FLOOR     2.0f                // minimum power required to consider a note

// ---------------------------------------------------------------------------
// Helper macro
// ---------------------------------------------------------------------------
#define count_of(x) (sizeof(x) / sizeof((x)[0]))

// ---------------------------------------------------------------------------
// Global Mode and Time Variables
// ---------------------------------------------------------------------------
typedef enum {
    MODE_NORMAL,
    MODE_SET_CLOCK,
    MODE_SET_ALARM
} Mode;

volatile Mode mode = MODE_NORMAL;

typedef struct {
    int hour;
    int min;
    int sec;
} ClockTime;

volatile ClockTime currentTime = {12, 0, 0};   // “real” clock time
volatile ClockTime alarmTime   = {7, 0, 0};      // alarm (wake‐up) time

volatile bool alarm_active = false;
absolute_time_t alarm_activation_time;  // used for LED blinking timing
volatile int current_sequence_index = 0;  // progress in the disarm sequence

// ---------------------------------------------------------------------------
// Musical key definitions and correct sequence for disarming
// ---------------------------------------------------------------------------
typedef struct {
    const char *name;
    float freq;
} Key;

Key keys[] = {
    {"C4", 261.63},
    {"D4", 293.66},
    {"E4", 329.63},
    {"F4", 349.23},
    {"G4", 392.00},
    {"A4", 440.00},
    {"B4", 493.88},
    {"C5", 523.25}
};
int num_keys = sizeof(keys) / sizeof(keys[0]);

// Define the required note sequence (here: C4, E4, G4)
#define SEQUENCE_LENGTH 3
int correct_sequence[SEQUENCE_LENGTH] = { 0, 2, 4 };

// ---------------------------------------------------------------------------
// Display buffer and render area (SSD1306 library)
// ---------------------------------------------------------------------------
uint8_t buf[SSD1306_BUF_LEN];
struct render_area frame_area;

// ---------------------------------------------------------------------------
// Goertzel algorithm to measure energy at a target frequency.
// Returns a normalized power value.
// ---------------------------------------------------------------------------
float goertzel(float *samples, float target_freq) {
    float omega = 2.0f * M_PI * target_freq / SAMPLE_RATE;
    float coeff = 2.0f * cosf(omega);
    float s_prev = 0.0f, s_prev2 = 0.0f, s;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        s = samples[i] + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }
    float power = s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;
    return power;
}

// ---------------------------------------------------------------------------
// detect_note()
// Samples the ADC and computes the energy at each key frequency.
// Returns the index of the key with the highest power (if above threshold)
// or -1 if nothing significant is detected.
// ---------------------------------------------------------------------------
int detect_note(void) {
    float samples[NUM_SAMPLES];
    float avg = 0.0f;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        samples[i] = (float)adc_read() / 4095.0f;
        avg += samples[i];
        sleep_us(SAMPLE_DELAY_US);
    }
    avg /= NUM_SAMPLES;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        samples[i] -= avg;
    }
    
    float max_power = 0.0f;
    int detected_index = -1;
    for (int i = 0; i < num_keys; i++) {
        float power = goertzel(samples, keys[i].freq);
        if (power > max_power) {
            max_power = power;
            detected_index = i;
        }
    }
    if (max_power > NOISE_FLOOR)
        return detected_index;
    else
        return -1;
}

// ---------------------------------------------------------------------------
// update_display()
// Updates the OLED with the current mode, clock time, alarm time,
// and, if the alarm is active, shows the next required note in the disarm sequence.
// ---------------------------------------------------------------------------
void update_display(void) {
    char line1[32], line2[32], line3[32], line4[32];
    const char *modeStr;
    
    switch (mode) {
        case MODE_NORMAL:    modeStr = "Normal";    break;
        case MODE_SET_CLOCK: modeStr = "Set Clock"; break;
        case MODE_SET_ALARM: modeStr = "Set Alarm"; break;
        default:             modeStr = "";
    }
    
    snprintf(line1, sizeof(line1), "Mode: %s", modeStr);
    snprintf(line2, sizeof(line2), "Time: %02d:%02d:%02d", currentTime.hour, currentTime.min, currentTime.sec);
    snprintf(line3, sizeof(line3), "Alarm: %02d:%02d", alarmTime.hour, alarmTime.min);
    
    if (alarm_active) {
        if (current_sequence_index < SEQUENCE_LENGTH)
            snprintf(line4, sizeof(line4), "ALARM! Next: %s", keys[correct_sequence[current_sequence_index]].name);
        else
            snprintf(line4, sizeof(line4), "ALARM! Code OK");
    } else {
        snprintf(line4, sizeof(line4), "");
    }
    
    memset(buf, 0, SSD1306_BUF_LEN);
    WriteString(buf, 0, 0, line1);
    WriteString(buf, 0, 8, line2);
    WriteString(buf, 0, 16, line3);
    WriteString(buf, 0, 24, line4);
    render(buf, &frame_area);
}

// ---------------------------------------------------------------------------
// Button IRQ handler
// If both buttons are pressed nearly simultaneously (within 100ms) the mode cycles.
// Otherwise, in a setting mode Button1 adjusts hours and Button2 adjusts minutes.
// ---------------------------------------------------------------------------
volatile absolute_time_t last_press_time_button1 = {0};
volatile absolute_time_t last_press_time_button2 = {0};

void gpio_irq_handler(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time();
    
    if (gpio == BUTTON_PIN1) {
        if (absolute_time_diff_us(last_press_time_button2, now) < 100000) {
            mode = (mode + 1) % 3;
        } else {
            if (mode == MODE_SET_CLOCK)
                currentTime.hour = (currentTime.hour + 1) % 24;
            else if (mode == MODE_SET_ALARM)
                alarmTime.hour = (alarmTime.hour + 1) % 24;
        }
        last_press_time_button1 = now;
    } else if (gpio == BUTTON_PIN2) {
        if (absolute_time_diff_us(last_press_time_button1, now) < 100000) {
            mode = (mode + 1) % 3;
        } else {
            if (mode == MODE_SET_CLOCK)
                currentTime.min = (currentTime.min + 1) % 60;
            else if (mode == MODE_SET_ALARM)
                alarmTime.min = (alarmTime.min + 1) % 60;
        }
        last_press_time_button2 = now;
    }
}

// ---------------------------------------------------------------------------
// main()
// Initialization of the display, LEDs, buttons, ADC, and audio.
// The main loop updates the clock, triggers the alarm when the set time is reached,
// and, when the alarm is active, repeatedly calls the audio function and checks for
// the correct note sequence to disarm the alarm.
// ---------------------------------------------------------------------------
int main(void) {
    stdio_init_all();
    setup_audio();  // initialize buzzer/audio logic

    // Initialize I2C for the OLED display.
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    SSD1306_init();
    frame_area.start_col  = 0;
    frame_area.end_col    = SSD1306_WIDTH - 1;
    frame_area.start_page = 0;
    frame_area.end_page   = SSD1306_NUM_PAGES - 1;
    calc_render_area_buflen(&frame_area);
    
    // Clear the display.
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);
    
    // Show welcome screen with scrolling.
    SSD1306_scroll(true);
    sleep_ms(5000);
    SSD1306_scroll(false);
    
    char *welcomeText[] = {
        "   Bem-Vindo ",
        " ao EmbarcaTech ",
        "      2024 ",
        "  SOFTEX/MCTI "
    };
    int y = 0;
    for (uint i = 0; i < count_of(welcomeText); i++) {
        WriteString(buf, 5, y, welcomeText[i]);
        y += 8;
    }
    render(buf, &frame_area);
    sleep_ms(3000);
    
    // Initialize LEDs.
    gpio_init(LED_PIN_RED);
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_put(LED_PIN_RED, 0);
    gpio_put(LED_PIN_GREEN, 0);
    
    // Initialize buttons with pull-ups.
    gpio_init(BUTTON_PIN1);
    gpio_init(BUTTON_PIN2);
    gpio_set_dir(BUTTON_PIN1, GPIO_IN);
    gpio_set_dir(BUTTON_PIN2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN1);
    gpio_pull_up(BUTTON_PIN2);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN2, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    // Initialize the ADC for note detection.
    adc_init();
    adc_gpio_init(ADC_INPUT_PIN);
    adc_select_input(ADC_CHANNEL);
    
    // Used for 1-second clock updates.
    absolute_time_t last_update = get_absolute_time();
    
    while (true) {
        update_display();
        absolute_time_t now = get_absolute_time();
        
        // Update the clock every second (unless in clock-set mode).
        if (mode != MODE_SET_CLOCK && absolute_time_diff_us(last_update, now) >= 1000000) {
            last_update = now;
            currentTime.sec++;
            if (currentTime.sec >= 60) {
                currentTime.sec = 0;
                currentTime.min++;
                if (currentTime.min >= 60) {
                    currentTime.min = 0;
                    currentTime.hour = (currentTime.hour + 1) % 24;
                }
            }
        }
        
        // Trigger the alarm when the current time equals the set alarm time.
        if (!alarm_active &&
            currentTime.hour == alarmTime.hour &&
            currentTime.min  == alarmTime.min &&
            currentTime.sec  == 0) {
            alarm_active = true;
            current_sequence_index = 0;  // Reset disarm sequence progress
            alarm_activation_time = now;
        }
        
        if (alarm_active) {
            // Continuously play the alarm sound.
            main_audio();
            
            // Blink the red LED.
            uint32_t ms = to_ms_since_boot(now);
            if ((ms / 500) % 2 == 0)
                gpio_put(LED_PIN_RED, 1);
            else
                gpio_put(LED_PIN_RED, 0);
            
            // Check for a played note to disarm the alarm.
            int detected = detect_note();
            if (detected != -1) {
                if (detected == correct_sequence[current_sequence_index]) {
                    current_sequence_index++;
                    sleep_ms(200);  // small debounce/delay
                    if (current_sequence_index >= SEQUENCE_LENGTH) {
                        // The full sequence was played correctly: stop the alarm.
                        alarm_active = false;
                        current_sequence_index = 0;
                        gpio_put(LED_PIN_RED, 0);
                    }
                } else {
                    // Wrong note: reset the sequence progress.
                    current_sequence_index = 0;
                }
            }
        } else {
            // Ensure the red LED is off when the alarm is inactive.
            gpio_put(LED_PIN_RED, 0);
        }
        
        // Use the green LED to indicate Normal mode.
        if (mode == MODE_NORMAL)
            gpio_put(LED_PIN_GREEN, 1);
        else
            gpio_put(LED_PIN_GREEN, 0);
        
        sleep_ms(100);
    }
    
    return 0;
}
