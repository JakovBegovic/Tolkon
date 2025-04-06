#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/dac.h"
#include "sound.h"

#define PI 3.14159
#define SAMPLE_RATE     1000
#define DAC_DELAY_MS    (1000 / SAMPLE_RATE)

typedef enum {
    SOUND_NONE,
    SOUND_VERY_CLOSE,
    SOUND_MID_CLOSE,
    SOUND_FAR,
    SOUND_VERY_FAR
} sound_event_t;

typedef struct {
    uint16_t frequency;
    uint16_t duration_ms;
} tone_command_t;

static QueueHandle_t sound_queue;
static QueueHandle_t sound_event_queue;

static void play_tone_blocking(uint16_t freq, uint16_t duration_ms)
{
    int samples = SAMPLE_RATE * duration_ms / 1000;
    for (int i = 0; i < samples; i++) {
        float theta = 2.0 * PI * freq * i / SAMPLE_RATE;
        uint8_t val = (uint8_t)((sin(theta) + 1.0f) * 127.5f);
        dac_output_voltage(DAC_CHANNEL_1, val);
        vTaskDelay(pdMS_TO_TICKS(DAC_DELAY_MS));
    }

    dac_output_voltage(DAC_CHANNEL_1, 0);
}

static void sound_tone_task(void *param)
{
    dac_output_enable(DAC_CHANNEL_1); // GPIO25 DAC1

    tone_command_t cmd;
    while (1) {
        if (xQueueReceive(sound_queue, &cmd, portMAX_DELAY)) {
            play_tone_blocking(cmd.frequency, cmd.duration_ms);
        }
    }
}

static void sound_event_task(void *pvParameters)
{
    sound_event_t evt;
    while (1) {
        if (xQueueReceive(sound_event_queue, &evt, portMAX_DELAY)) {
            switch (evt) {
                case SOUND_VERY_CLOSE:
                    for (int i = 0; i < 3; i++) {
                        sound_play(880, 100);
                        vTaskDelay(pdMS_TO_TICKS(150));
                    }
                    break;
                case SOUND_MID_CLOSE:
                    for (int i = 0; i < 2; i++) {
                        sound_play(660, 150);
                        vTaskDelay(pdMS_TO_TICKS(300));
                    }
                    break;
                case SOUND_FAR:
                    sound_play(523, 200);
                    vTaskDelay(pdMS_TO_TICKS(600));
                    break;
                case SOUND_VERY_FAR:
                    sound_play(440, 100);
                    vTaskDelay(pdMS_TO_TICKS(800));
                    break;
                default:
                    break;
            }
        }
    }
}

void sound_init(void)
{
    sound_queue = xQueueCreate(4, sizeof(tone_command_t));
    xTaskCreate(sound_tone_task, "sound_tone_task", 2048, NULL, 5, NULL);

    sound_event_queue = xQueueCreate(4, sizeof(sound_event_t));
    xTaskCreate(sound_event_task, "sound_event_task", 2048, NULL, 5, NULL);
}

void sound_play(uint16_t freq, uint16_t duration_ms)
{
    tone_command_t cmd = {
        .frequency = freq,
        .duration_ms = duration_ms
    };
    xQueueSend(sound_queue, &cmd, 0);
}

void sound_send_distance_event(bool flag1_close, bool flag2_midclose, bool flag3_far, bool flag4_veryfar)
{
    sound_event_t evt = SOUND_NONE;

    if (flag1_close)
        evt = SOUND_VERY_CLOSE;
    else if (flag2_midclose)
        evt = SOUND_MID_CLOSE;
    else if (flag3_far)
        evt = SOUND_FAR;
    else if (flag4_veryfar)
        evt = SOUND_VERY_FAR;

    xQueueSend(sound_event_queue, &evt, 0);
}
