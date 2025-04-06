SET(SOURCES screens/ui_Home_Scr.c
    screens/ui_Parking_assistant.c
    screens/ui_Warning_Scr.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_wifi_2_yes_png.c
    images/ui_img_auto_2_png.c
    images/ui_img_linije_2_png.c
    images/ui_img_linije_2_png.c)

add_library(ui ${SOURCES})
