#pragma once

#include "led_helpers.h"

const int PIN_LOGO_DATA = 21;
const int PIN_LOGO_CLOCK = 3;
const int PIN_BASE_DATA = 19;
const int PIN_BASE_CLOCK = 18;
const int PIN_BACK_SPOT = 26;
const int PIN_FLOOR_SPOT = 27;

void overwriteConfigForTrophy(JsonObject doc)
{
    // Usermods usually only care about their own stuff, but we go for violating boundaries here.
    JsonObject led = doc[F("hw")][F("led")];
    auto outputs = led[F("ins")];
    outputs.clear();
    auto logo = outputs.createNestedObject();
    auto base = outputs.createNestedObject();
    auto spotBack =  outputs.createNestedObject();
    auto spotFloor = outputs.createNestedObject();

    // qm210: take care that each "start" is continuous.
    int n = 0;
    configTrophyLedStripe(logo, n, 106, PIN_LOGO_DATA, PIN_LOGO_CLOCK);
    n += 106;
    configTrophyLedStripe(base, n, 64, PIN_BASE_DATA, PIN_BASE_CLOCK);
    n += 64;
    configTrophySinglePwm(spotBack, n, PIN_BACK_SPOT);
    n++;
    configTrophySinglePwm(spotFloor, n, PIN_FLOOR_SPOT);
    n++;
    led[F("total")] = n; // is unused, I believe, but anyway.

    // The LED setup is completely hardcoded by now,
    // but still, we don't want any further auto-segmentation
    doc[F("light")][F("aseg")] = false;
}
