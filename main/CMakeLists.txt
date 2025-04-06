set(SRCS "app_main.c"
         "wifi_controller.c"
         "ultrasonic.c"
         "sound.c"
         "i2c_control.c")

set(INCLUDES ".")

idf_component_register(SRCS ${SRCS}
                       INCLUDE_DIRS ${INCLUDES})
