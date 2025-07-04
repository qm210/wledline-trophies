#include "DeadlineTrophy.h"

namespace DeadlineTrophy {

    uint16_t maxMilliAmps(uint16_t nLeds) {
        // from wled_cfg(2).json (should be in this repo) - set via #define in the DeadlineTrophy.h
        // "maxpwr": maximum 1200 mA total (= ABL_MILLIAMPS_DEFAULT = BusManager::ablMilliampsMax())
        // "ledma":  maximum 255 mA per LED (= LED_MILLIAMPS_DEFAULT)
        return (BusManager::ablMilliampsMax() * nLeds) / N_LEDS_TOTAL;
    }

    BusConfig createBus(uint8_t type, uint16_t count, uint16_t start, uint8_t pin1, uint8_t pin2) {
        const uint8_t colorOrder = COL_ORDER_RGB; // QM-REVERT: COL_ORDER_GRB, but see whether the WebSocket Peek works now
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
            count,
            colorOrder,
            reversed,
            skipFirst,
            AWmode,
            freqHz,
            LED_MILLIAMPS_DEFAULT,
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
            N_LEDS_BASE,
            start,
            PIN_BASE_DATA,
            PIN_BASE_CLOCK
        ));
        start += N_LEDS_BASE;
        busConfigs.emplace_back(createBus(
            LED_TYPE_HD107S,
            N_LEDS_LOGO,
            start,
            PIN_LOGO_DATA,
            PIN_LOGO_CLOCK
        ));
        start += N_LEDS_LOGO;
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

        gammaCorrectBri = false;
        gammaCorrectCol = true;
        gammaCorrectVal = 2.8;
        NeoGammaWLEDMethod::calcGammaTable(gammaCorrectVal);

        #ifdef DEADLINE_INIT_BRIGHTNESS
        briS = DEADLINE_INIT_BRIGHTNESS;
        turnOnAtBoot = briS > 0;
        #else
        briS = 96;
        turnOnAtBoot = true;
        #endif

        transitionDelayDefault = 100;
        transitionDelay = transitionDelayDefault;
        blendingStyle = 0;
    }

    const char* segmentName[] = {
        "Base Square",
        "Deadline Logo",
        "Back Spot",
        "Floor Spot"
    };
    const Segment segment[] = {
        {
            0,
            DeadlineTrophy::baseEdge,
            DeadlineTrophy::logoH,
            DeadlineTrophy::logoH + DeadlineTrophy::baseEdge
        },
        {
            0,
            DeadlineTrophy::logoW,
            0,
            DeadlineTrophy::logoH
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