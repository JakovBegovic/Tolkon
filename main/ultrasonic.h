#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdbool.h>

void ultrasonic_init(void);

extern volatile bool flag1_close;
extern volatile bool flag2_midclose;
extern volatile bool flag3_far;
extern volatile bool flag4_veryfar;
void update_senzor_flags(bool flag1_close, bool flag2_midclose, bool flag3_far, bool flag4_veryfar);

#endif // ULTRASONIC_H
