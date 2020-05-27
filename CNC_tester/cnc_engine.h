#ifndef CNC_ENGINE_H
#define CNC_ENGINE_H

#include <stdbool.h>
#include <stdint.h>

enum
{
    CNC_AXIS_X,
    CNC_AXIS_Y,
    CNC_AXIS_Z,

    CNC_AXIS_COUNT,
};

void cnc_engine_init(void);
void cnc_engine_enable(bool state);
void cnc_engine_enable_single(int axis, bool state);

#endif // CNC_ENGINE_H