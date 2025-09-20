#include "DeadlineTrophy.h"

namespace DeadlineTrophy {

    uint16_t maxMilliAmps(uint16_t nLeds) {
        // from wled_cfg(2).json (should be in this repo) - set via #define in the DeadlineTrophy.h
        // "maxpwr": maximum 1200 mA total (= ABL_MILLIAMPS_DEFAULT = BusManager::ablMilliampsMax())
        // "ledma":  maximum 255 mA per LED (= LED_MILLIAMPS_DEFAULT)
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
        // Usermods usually only care about their own stuff, but then again:
        // "You're remembered for the rules you break" - Stockton Rush
        DEBUG_PRINTLN(F("[USE_DEADLINE_CONFIG] Overwrite config by Deadline Trophy hard-coded values."));

        strip.isMatrix = true;

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

        // QM: let's keep the gamma functioning right now,
        // but we should fix it & communicate that before the compo
        // gammaCorrectBri = false;
        // gammaCorrectCol = true;
        // gammaCorrectVal = 1.0; // <-- messy suggested, default was 2.7 or something
        // NeoGammaWLEDMethod::calcGammaTable(gammaCorrectVal);

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
        { 0, baseSize, logoH, logoH + baseSize },
        { 0, logoW, 0, logoH },
        { baseSize, baseSize + 1, logoH, logoH + 1 },
        { baseSize, baseSize + 1, logoH + 1, logoH + 2 }
    };
    const uint8_t segmentCapabilities[] = {
        SEG_CAPABILITY_RGB,
        SEG_CAPABILITY_RGB,
        SEG_CAPABILITY_W,
        SEG_CAPABILITY_W
    };

    static std::array<NormalizedCoord, N_LEDS_LOGO> logoCoordinates_;
    static bool logoInitialized = false;

    std::array<NormalizedCoord, N_LEDS_LOGO>& logoCoordinates() {
        if (logoInitialized) {
            return logoCoordinates_;
        }
        float dx = 1. / static_cast<float>(logoW);
        float dy = 1. / static_cast<float>(logoH);
        float x0 = -0.5f + (logoW % 2 == 0 ? dx : 0.f);
        float y0 = -0.5f + (logoH % 2 == 0 ? dy : 0.f);
        for (uint8_t x = 0; x < logoW; ++x)
        for (uint8_t y = 0; y < logoH; ++y) {
            uint8_t ledIndex = mappingTable[x + logoW * y];
            uint8_t ledIndexInLogo = ledIndex - N_LEDS_BASE;
            if (ledIndex > N_LEDS_TOTAL) {
                continue;
            }
            logoCoordinates_[ledIndexInLogo] = {
                static_cast<float>(x) * dx + x0,
                static_cast<float>(y) * dy + y0,
            };
        }
        logoInitialized = true;
        return logoCoordinates_;
    }

}
