#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "ultrasonic.h"
#include "ui.h"
#include "sound.h"


#define TRIG_GPIO GPIO_NUM_27
#define ECHO_GPIO GPIO_NUM_26

extern lv_obj_t * ui_senzorFlag1;
extern lv_obj_t * ui_senzorFlag2;
extern lv_obj_t * ui_senzorFlag3;
extern lv_obj_t * ui_senzorFlag4;

volatile bool flag1_close = false;
volatile bool flag2_midclose = false;
volatile bool flag3_far = false;
volatile bool flag4_veryfar = false;

static TaskHandle_t ultrasonic_task_handle = NULL;
static TimerHandle_t ultrasonic_timer = NULL;

static void ultrasonic_timer_callback( TimerHandle_t xTimer )
{
    if ( ultrasonic_task_handle != NULL )
    {
        xTaskNotifyGive( ultrasonic_task_handle );
    }
}

static void ultrasonic_task( void *pvParameters )
{
    gpio_set_direction( TRIG_GPIO, GPIO_MODE_OUTPUT );
    gpio_set_direction( ECHO_GPIO, GPIO_MODE_INPUT );

    while ( 1 )
    {
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY ); // Čekaj signal od timera

        // Trigger pulse
        gpio_set_level( TRIG_GPIO, 0 );
        esp_rom_delay_us( 2 );
        gpio_set_level( TRIG_GPIO, 1 );
        esp_rom_delay_us( 10 );
        gpio_set_level( TRIG_GPIO, 0 );

        int64_t start_time = 0;
        int64_t end_time = 0;
        int64_t timeout = 0;
        float distance_cm = -1;

        // Čekaj početak ECHO signala (HIGH)
        timeout = esp_timer_get_time() + 30000;

        while ( gpio_get_level( ECHO_GPIO ) == 0 )
        {
            if ( esp_timer_get_time() > timeout )
            {
                distance_cm = -1;
                break;
            }
        }

        start_time = esp_timer_get_time();

        // Čekaj kraj ECHO signala (LOW)
        timeout = esp_timer_get_time() + 30000;

        while ( gpio_get_level( ECHO_GPIO ) == 1 )
        {
            if ( esp_timer_get_time() > timeout )
            {
                distance_cm = -1;
                break;
            }
        }

        end_time = esp_timer_get_time();

        // Ako je sve u redu, izračunaj trajanje pulsa i udaljenost
        if ( end_time > start_time )
        {
            int64_t pulse_duration = end_time - start_time;

            // Ako je trajanje preveliko, znači da je udaljenost > 60 cm
            if ( pulse_duration > 3500 ) // ~60 cm = 3500 µs
            {
                distance_cm = 60.0;
            }
            else
            {
                distance_cm = ( pulse_duration * 0.0343 ) / 2.0;
            }
        }
        else
        {
            // Ako nije validno mjerenje
            distance_cm = 60.0;
        }


        // Postavi flagove i zvuk
        if ( distance_cm > 0 && distance_cm < 400 )
        {
            flag1_close = ( distance_cm < 10 );
            flag2_midclose = ( distance_cm < 20 );
            flag3_far = ( distance_cm < 30 );
            flag4_veryfar = ( distance_cm < 40 );
        }
        else
        {
            flag1_close = flag2_midclose = flag3_far = flag4_veryfar = false;
        }

        update_senzor_flags( flag1_close, flag2_midclose, flag3_far, flag4_veryfar );
        sound_send_distance_event( flag1_close, flag2_midclose, flag3_far, flag4_veryfar );

        if ( distance_cm > 0 )
        {
            printf( "Distance: %.2f cm | F1:%d F2:%d F3:%d F4:%d\n",
                    distance_cm, flag1_close, flag2_midclose, flag3_far, flag4_veryfar );
        }
        else
        {
            printf( "Distance: INVALID | F1:0 F2:0 F3:0 F4:0\n" );
        }
    }
}



void update_senzor_flags( bool flag1_close, bool flag2_midclose, bool flag3_far, bool flag4_veryfar )
{
    if ( flag1_close )
    {
        lv_obj_clear_flag( ui_senzorFlag1, LV_OBJ_FLAG_HIDDEN );
    }
    else
    {
        lv_obj_add_flag( ui_senzorFlag1, LV_OBJ_FLAG_HIDDEN );
    }

    if ( flag2_midclose )
    {
        lv_obj_clear_flag( ui_senzorFlag2, LV_OBJ_FLAG_HIDDEN );
    }
    else
    {
        lv_obj_add_flag( ui_senzorFlag2, LV_OBJ_FLAG_HIDDEN );
    }

    if ( flag3_far )
    {
        lv_obj_clear_flag( ui_senzorFlag3, LV_OBJ_FLAG_HIDDEN );
    }
    else
    {
        lv_obj_add_flag( ui_senzorFlag3, LV_OBJ_FLAG_HIDDEN );
    }

    if ( flag4_veryfar )
    {
        lv_obj_clear_flag( ui_senzorFlag4, LV_OBJ_FLAG_HIDDEN );
    }
    else
    {
        lv_obj_add_flag( ui_senzorFlag4, LV_OBJ_FLAG_HIDDEN );
    }
}

void ultrasonic_init( void )
{
    xTaskCreate( ultrasonic_task, "ultrasonic_task", 2048, NULL, 6, &ultrasonic_task_handle );

    ultrasonic_timer = xTimerCreate( "ultrasonic_timer", pdMS_TO_TICKS( 500 ), pdTRUE, NULL, ultrasonic_timer_callback );
    xTimerStart( ultrasonic_timer, 0 );
}