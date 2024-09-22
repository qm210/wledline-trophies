#pragma once

#include "led_helpers.h"

const int PIN_LOGO_DATA = 19;
const int PIN_LOGO_CLOCK = 18;
const int PIN_BASE_DATA = 3;
const int PIN_BASE_CLOCK = 21;
const int PIN_UV = 26;
const int PIN_WHITE = 27;

void overwriteConfigForTrophy(JsonObject& doc)
{
    // Usermods usually only care about their own stuff, but we go for violating boundaries here.
    JsonObject led = doc["hw"]["led"];

    auto outputs = led.createNestedArray("ins");
    // qm210: if you change these, take care that the "start" is continuous...
    auto logo = trophyLedStripe(outputs, 0, 106, PIN_LOGO_DATA, PIN_LOGO_CLOCK);
    auto base = trophyLedStripe(outputs, 106, 64, PIN_BASE_DATA, PIN_BASE_CLOCK);
    auto uv = trophySinglePwm(outputs, 170, PIN_UV);
    auto white = trophySinglePwm(outputs, 171, PIN_WHITE);

    outputs.add(logo);
    outputs.add(base);
    outputs.add(uv);
    outputs.add(white);

    // PS: AudioReactive settings are defined via -D compiler flags (cf. audio_reactive.h)
}
