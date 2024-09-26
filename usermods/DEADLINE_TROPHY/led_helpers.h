#pragma once

#include "wled.h"

void configTrophyBaseOutput(JsonObject output) {
    // this misses "type", "start", "end", "freq" and all the "pin"s
    output.createNestedArray("pin");
    output["order"] = 0;
    output["rev"] = false;
    output["skip"] = 0;
    output["ref"] = false;
    output["rgbwm"] = 0;
}

void configTrophyLedStripe(JsonObject output, int start, int len, int dataPin, int clockPin) {
    configTrophyBaseOutput(output);
    output["type"] = 51; // HD107S-2020 is compatible to the APA102 type
    output["start"] = start;
    output["len"] = len;
    output["pin"].add(dataPin);
    output["pin"].add(clockPin);
    output["freq"] = 5000; // "Normal"
}

void configTrophySinglePwm(JsonObject output, int start, int dataPin) {
    configTrophyBaseOutput(output);
    output["type"] = 41; // "white pwm"
    output["start"] = start;
    output["len"] = 1;
    output["pin"].add(dataPin);
    output["freq"] = 6510;
}
