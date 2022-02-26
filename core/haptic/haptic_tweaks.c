/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <limits.h>
#include <haptic/haptic_core.h>
#include "gimx.h"

#define SWAP(TYPE, V1, V2) \
        TYPE tmp = V1; \
        V1 = V2; \
        V2 = tmp;

#define CLAMP(MIN,VALUE,MAX) (((VALUE) < MIN) ? (MIN) : (((VALUE) > MAX) ? (MAX) : (VALUE)))

#define APPLY_GAIN(VALUE, GAIN, MIN, MAX) \
        { \
            int64_t tmp = VALUE * GAIN / 100; \
            VALUE = CLAMP(MIN, tmp, MAX); \
        }

void haptic_tweak_apply(const s_haptic_core_tweaks * tweaks, s_haptic_core_data * data) {

    switch (data->type) {
    case E_DATA_TYPE_RUMBLE:
        if (tweaks->gain.rumble != 100) {
            APPLY_GAIN(data->rumble.weak, tweaks->gain.rumble, 0, USHRT_MAX)
            APPLY_GAIN(data->rumble.strong, tweaks->gain.rumble, 0, USHRT_MAX)
        }
        if (tweaks->invert) {
            SWAP(uint16_t, data->rumble.weak, data->rumble.strong)
        }
        break;
    case E_DATA_TYPE_CONSTANT:
        // get the axis value
        if(tweaks->axis_address) {
            int axis_value = *(tweaks->axis_address);
            int constant_level = data->constant.level;
            
            // calculate the gain
            if(abs(axis_value) < 300) {
                int gain = abs(axis_value) / 3;
                APPLY_GAIN(data->constant.level, gain, -SHRT_MAX, SHRT_MAX);
            }
            ginfo("haptic_tweak_apply: axis address: %p, axis value: %d, constant_level: %d, tweaked_constant_level\n", (void *)tweaks->axis_address, axis_value, constant_level, data->constant.level);

        }
        if (tweaks->gain.constant != 100) {
            APPLY_GAIN(data->constant.level, tweaks->gain.constant, -SHRT_MAX, SHRT_MAX)
        }
        if (tweaks->invert) {
            data->constant.level = -data->constant.level;
        }
        break;
    case E_DATA_TYPE_SPRING:
        if (tweaks->gain.spring != 100) {
            APPLY_GAIN(data->spring.saturation.left, tweaks->gain.spring, 0, USHRT_MAX)
            APPLY_GAIN(data->spring.saturation.right, tweaks->gain.spring, 0, USHRT_MAX)
            APPLY_GAIN(data->spring.coefficient.left, tweaks->gain.spring, -SHRT_MAX, SHRT_MAX)
            APPLY_GAIN(data->spring.coefficient.right, tweaks->gain.spring, -SHRT_MAX, SHRT_MAX)
        }
        if (tweaks->invert) {
            SWAP(int16_t, data->spring.coefficient.left, data->spring.coefficient.right)
            data->spring.center = -data->spring.center;
        }
        break;
    case E_DATA_TYPE_DAMPER:
        if (tweaks->gain.damper != 100) {
            APPLY_GAIN(data->damper.saturation.left, tweaks->gain.damper, 0, USHRT_MAX)
            APPLY_GAIN(data->damper.saturation.right, tweaks->gain.damper, 0, USHRT_MAX)
            APPLY_GAIN(data->damper.coefficient.left, tweaks->gain.damper, -SHRT_MAX, SHRT_MAX)
            APPLY_GAIN(data->damper.coefficient.right, tweaks->gain.damper, -SHRT_MAX, SHRT_MAX)
        }
        if (tweaks->invert) {
            SWAP(int16_t, data->spring.coefficient.left, data->spring.coefficient.right)
        }
        break;
    case E_DATA_TYPE_LEDS:
        break;
    case E_DATA_TYPE_RANGE:
        break;
    case E_DATA_TYPE_NONE:
        break;
    }
}
