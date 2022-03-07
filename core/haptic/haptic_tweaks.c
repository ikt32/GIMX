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
        if (tweaks->g29.enable) {
            int constant_level = data->constant.level;

            int min_gain = tweaks->g29.min_gain;
            int range_start = tweaks->g29.range_start;
            int range_end = tweaks->g29.range_end;
            int ffb_range = range_end - range_start;
            
            // calculate the gain within the FFB range
            // use a cosine curve between the upper and lower bounds
            // to help smooth the transitions

            int abs_constant = abs(constant_level);
            int clamped = CLAMP(range_start, abs_constant, range_end);

            double r = ( (ffb_range - (clamped - range_start)) * M_PI) / ffb_range;
            double c = ( cos(r) + 1.0 ) * (100.0 - min_gain) / 2.0;
            int gain = (int)c + min_gain;

            APPLY_GAIN(data->constant.level, gain, -SHRT_MAX, SHRT_MAX);
            if(gimx_params.debug.haptic) {
                gtime now = gtime_gettime();
                printf("g29_correction: time: %lu.%06lu, constant_level: %d, gain %d\n",
                     GTIME_SECPART(now), GTIME_USECPART(now), constant_level, gain);
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
