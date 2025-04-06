#pragma once

#include <stdint.h>
#include <stdbool.h>

void sound_init(void);
void sound_play(uint16_t freq, uint16_t duration_ms);
void sound_send_distance_event(bool flag1_close, bool flag2_midclose, bool flag3_far, bool flag4_veryfar);
