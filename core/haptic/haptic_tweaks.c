/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <limits.h>
#include <haptic/haptic_core.h>
#include <math.h>
#include <gimx.h>
#include <gimxtime/include/gtime.h>

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

#define PI 3.14159265

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
        if(tweaks->g29.enable && tweaks->g29.axis_address) {
            int axis_value = *(tweaks->g29.axis_address);
            int constant_level = data->constant.level;

            int zero_gain = tweaks->g29.zero_gain;
            int axis_range = tweaks->g29.axis_range;
            int gain = 100;
            
            // calculate the gain
            if(abs(axis_value) < axis_range) {
                // calculate the gain within the axis range
                // use a cosine curve between the upper and lower bounds
                // to help smooth the transitions

                double r = ((axis_range - abs(axis_value)) * PI) / axis_range;
                double c = ( cos(r) + 1.0 ) * (100.0 - zero_gain) / 2.0;
                gain = (int)c + zero_gain;

                //gain = (abs(axis_value) * (100 - zero_gain) / axis_range) + zero_gain;
                APPLY_GAIN(data->constant.level, gain, -SHRT_MAX, SHRT_MAX);
            }
            if(gimx_params.debug.haptic) {
                gtime now = gtime_gettime();
                printf("g29_correction: time: %lu.%06lu, axis address: %p, axis value: %d, constant_level: %d, gain %d\n",
                     GTIME_SECPART(now), GTIME_USECPART(now),
                    (void *)tweaks->g29.axis_address, axis_value, constant_level, gain);
            }
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
