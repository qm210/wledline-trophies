#pragma once

#include "led_helpers.h"

const int PIN_LOGO_DATA = 21;
const int PIN_LOGO_CLOCK = 3;
const int PIN_BASE_DATA = 19;
const int PIN_BASE_CLOCK = 18;
const int PIN_UV = 26;
const int PIN_WHITE = 27;

void overwriteConfigForTrophy(JsonObject doc)
{
    // Usermods usually only care about their own stuff, but we go for violating boundaries here.
    JsonObject led = doc[F("hw")][F("led")];
    auto outputs = led[F("ins")];
    outputs.clear();
    auto logo = outputs.createNestedObject();
    auto base = outputs.createNestedObject();
    auto uv =  outputs.createNestedObject();
    auto white = outputs.createNestedObject();

    // qm210: if you change these, take care that each "start" is continuous.
    configTrophyLedStripe(logo, 0, 106, PIN_LOGO_DATA, PIN_LOGO_CLOCK);
    configTrophyLedStripe(base, 106, 64, PIN_BASE_DATA, PIN_BASE_CLOCK);
    configTrophySinglePwm(uv, 170, PIN_UV);
    configTrophySinglePwm(white, 171, PIN_WHITE);
    led[F("total")] = 106 + 64 + 1 + 1;

    // The LED setup is completely hardcoded by now,
    // so probably even the autoSegment is obsolete - but well, why not.
    doc[F("light")][F("aseg")] = true;
}
