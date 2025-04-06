#include <stdio.h>
#include <string.h>
#include <time.h>
#include "gui/gui.h"
#include "lvgl.h"
#include "ui.h"
#include "ultrasonic.h"
#include "sound.h"
#include "wifi_controller.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_mac.h"

#include "../components/veml7700-esp-idf/include/veml7700.h"

#define SCALE_CELCIUS   'C'
#define SCALE_FAHRENHEIT 'F'
#define SCALE_KELVIN    'K'

#define UART_STACK_SIZE 4096
#define INIT_DELAY      500
#define SLEEP_DELAY     5000

#define I2C_MASTER_SDA_IO 32
#define I2C_MASTER_SCL_IO 14
#define I2C_MASTER_FREQ_HZ 100000

const int i2cMasterNum = I2C_NUM_0;
static const char *MAIN_TAG = "main";
static const char *SENSORS_TAG = "sensors";

char strftime_buf[ 64 ];
veml7700_handle_t sensor;

static void i2c_init_master();

// ⬇️ Task koji ažurira vrijeme na GUI
static void time_update_task( void *pvParameters )
{
    struct tm timeinfo;
    char strftime_buf[ 64 ];

    while ( 1 )
    {
        time_t now;
        time(&now );
        localtime_r(&now, &timeinfo );

        strftime( strftime_buf, sizeof( strftime_buf ), "%a %d %B %H:%M", &timeinfo );
        lv_label_set_text( ui_Time, strftime_buf );
        lv_label_set_text( ui_Time1, strftime_buf );
        vTaskDelay( 60000U / portTICK_PERIOD_MS );
    }
}

static void light_update_task( void *pvParameters )
{
    double lux = 0;

    while ( 1 )
    {
        if ( veml7700_read_als_lux_auto( sensor, &lux ) == ESP_OK )
        {
            printf( "Ambient Light: %.2f lux\n", lux );
            char text[ 50 ]; // Make sure the buffer is large enough

            sprintf( text, "%.2f lux\n", lux );

            lv_label_set_text( ui_Temp1, text );
            lv_label_set_text( ui_Temp, text );
        }
        else
        {
            printf( "Failed to read light level\n" );
        }

        vTaskDelay( 300U / portTICK_PERIOD_MS );
    }
}


void app_main( void )
{

    // ✅ Inicijalizacije
    wifi_init_sta();
    gui_init();
    setup_time();
    ultrasonic_init();
    sound_init();

    // Initialize lux sensor
    i2c_init_master();

    if ( veml7700_initialize(&sensor, i2cMasterNum ) != ESP_OK )
    {
        printf( "Failed to initialize VEML7700 sensor" );
        return;
    }

    // ✅ Taskovi
    xTaskCreate( time_update_task, "time_update_task", 4096, NULL, 5, NULL );
    xTaskCreate( light_update_task, "light_update_task", 4096, NULL, 5, NULL );

    for( ;; )
    {
        vTaskDelay( 1000U / portTICK_PERIOD_MS );
    }

}

static void i2c_init_master()
{
    i2c_config_t conf =
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config( i2cMasterNum, &conf );
    i2c_driver_install( i2cMasterNum, conf.mode, 0, 0, 0 );
}