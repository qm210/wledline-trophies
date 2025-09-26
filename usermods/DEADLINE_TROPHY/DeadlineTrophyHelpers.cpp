#include "DeadlineTrophy.h"

namespace DeadlineTrophy {

    namespace FxHelpers {

        uint32_t floatHSV(float hue, float sat, float val) {
            // parameters in [0, 255] but as float
            uint32_t color = uint32_t(CRGB(CHSV(
                static_cast<uint8_t>(hue),
                static_cast<uint8_t>(sat),
                static_cast<uint8_t>(val)
            )));
            // remove the specific "white" that might be in the color as no Trophy pixel has a white
            return color & 0x00FFFFFF;
        }

        FloatRgb cosinePalette(float t, FloatRgb offset, FloatRgb scale, FloatRgb frequency, FloatRgb phase) {
            // useful tool right here: https://dev.thi.ng/gradients/
            float r = offset.r + scale.r * cos_t(M_TWOPI * (frequency.r * t + phase.r));
            float g = offset.g + scale.g * cos_t(M_TWOPI * (frequency.g * t + phase.g));
            float b = offset.b + scale.b * cos_t(M_TWOPI * (frequency.b * t + phase.b));
            return {clip(r), clip(g), clip(b)};
        }

        float invSqrt(float x) {
            // the Quake III Arena implementation, to compare against direct sqrtf() and/or sqrt32_bw() with casting
            uint32_t i;
            float x2, y;
            const float threehalfs = 1.5F;
            x2 = x * 0.5F;
            y = x;
            memcpy(&i, &y, sizeof(float)); // i = *(long*)&y;
            i = 0x5f3759df - (i >> 1);
            memcpy(&y, &i, sizeof(uint32_t)); // y = *(float*)&i;
            y = y * (threehalfs - (x2 * y * y));
            return y;
        }

        uint8_t mix8(uint8_t a, uint8_t b, float t) {
            return a + static_cast<uint8_t>(static_cast<float>(b - a) * t);
        }

        uint32_t mixRgb(uint32_t color1, uint32_t color2, float t) {
            // yes, RGB mixing is ugly, but that's the easiest to get right here
            uint8_t r = mix8(R(color1), R(color2), t);
            uint8_t g = mix8(G(color1), G(color2), t);
            uint8_t b = mix8(B(color1), B(color2), t);
            return (r << 16) | (g << 8) | b;
        }

        uint8_t pow8(uint8_t base, float exponent) {
            if (base == 0) {
                return 0;
            }
            float value = static_cast<float>(base) / 255.;
            return static_cast<uint8_t>(255. * exp(exponent * logf(value)));
        }

        uint8_t scale8f(uint8_t val, float factor) {
            return static_cast<uint8_t>(factor * static_cast<float>(val));
        }

        CRGB& scale(CRGB& color, float factor) {
            color.r = scale8f(color.r, factor);
            color.g = scale8f(color.g, factor);
            color.b = scale8f(color.b, factor);
            return color;
        }

        CRGB& grade(CRGB& color, float exponent) {
            color.r = pow8(color.r, exponent);
            color.g = pow8(color.g, exponent);
            color.b = pow8(color.b, exponent);
            return color;
        }

        long measureMicros() {
            static int64_t t0 = 0, t1 = 0;
            int64_t delta = -1;
            if (t0 != 0) {
                t1 = esp_timer_get_time();
                delta = static_cast<long>(t1 - t0);
            }
            t0 = esp_timer_get_time();
            return delta;
        }
    }

}
