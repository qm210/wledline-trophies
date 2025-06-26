#include "DeadlineTrophy.h"

namespace DeadlineTrophy {

    uint16_t maxMilliAmps(uint16_t nLeds) {
        // QM_WIP: should reference DEADLINE_MAX_AMPERE / ABL_MILLIAMPS_DEFAULT / LED_MILLIAMPS_DEFAULT?
        return (BusManager::ablMilliampsMax() * nLeds) / N_LEDS_TOTAL;
    }

    BusConfig createBus(uint8_t type, uint16_t count, uint16_t start, uint8_t pin1, uint8_t pin2) {
        const uint8_t colorOrder = COL_ORDER_GRB;
        const uint8_t skipFirst = 0;
        const bool reversed = false;
        const uint8_t AWmode = RGBW_MODE_MANUAL_ONLY;
        const uint16_t freqHz_normal = 5000; // called "Normal" in the UI
        const uint16_t freqHz_pwmWhite = 6510;
        uint16_t freqHz = type == LED_SINGLE_WHITE
            ? freqHz_pwmWhite
            : freqHz_normal;
        uint8_t pins[] = {pin1, pin2};
        return BusConfig(
            type,
            pins,
            start,
            N_LEDS_LOGO,
            colorOrder,
            reversed,
            skipFirst,
            AWmode,
            freqHz,
            maxMilliAmps(1),
            maxMilliAmps(count)
        );
    }

    void overwriteConfig()
    {
        // Usermods usually only care about their own stuff, but:
        // "You're remembered for the rules you break" - Stockton Rush
        DEBUG_PRINTLN(F("[USE_DEADLINE_CONFIG] Overwrite config by Deadline Trophy hard-coded values."));

        strip.setMatrix(true);

        busConfigs.clear();
        int start = 0;
        busConfigs.emplace_back(createBus(
            LED_TYPE_HD107S,
            N_LEDS_LOGO,
            start,
            PIN_LOGO_DATA,
            PIN_LOGO_CLOCK
        ));
        start += N_LEDS_LOGO;
        busConfigs.emplace_back(createBus(
            LED_TYPE_HD107S,
            N_LEDS_BASE,
            start,
            PIN_LOGO_DATA,
            PIN_LOGO_CLOCK
        ));
        start += N_LEDS_BASE;
        busConfigs.emplace_back(createBus(
            LED_SINGLE_WHITE,
            1,
            start,
            PIN_BACK_SPOT
        ));
        start += 1;
        busConfigs.emplace_back(createBus(
            LED_SINGLE_WHITE,
            1,
            start,
            PIN_FLOOR_SPOT
        ));

        #ifdef DEADLINE_INIT_BRIGHTNESS
        briS = DEADLINE_INIT_BRIGHTNESS
        turnOnAtBoot = briS > 0;
        #endif
    }

    const char* segmentName[] = {
        "Logo",
        "Base",
        "Back Spot",
        "Floor Spot"
    };
    const Segment segment[] = {
        {
            0,
            DeadlineTrophy::logoW,
            0,
            DeadlineTrophy::logoH
        },
        {
            0,
            DeadlineTrophy::baseEdge,
            DeadlineTrophy::logoH,
            DeadlineTrophy::logoH + DeadlineTrophy::baseEdge
        },
        {
            DeadlineTrophy::baseEdge,
            DeadlineTrophy::baseEdge + 1,
            DeadlineTrophy::logoH,
            DeadlineTrophy::logoH + 1
        },
        {
            DeadlineTrophy::baseEdge,
            DeadlineTrophy::baseEdge + 1,
            DeadlineTrophy::logoH + 1,
            DeadlineTrophy::logoH + 2
        }
    };
    const uint8_t segmentCapabilities[] = {
        SEG_CAPABILITY_RGB,
        SEG_CAPABILITY_RGB,
        SEG_CAPABILITY_W,
        SEG_CAPABILITY_W
    };
}