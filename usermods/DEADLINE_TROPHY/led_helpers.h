#pragma once

#include "wled.h"

JsonObject trophyBaseOutput(JsonArray outputs) {
    auto result = outputs.createNestedObject();
    // this misses "type", "start", "end", "freq" and all the "pin"s
    result.createNestedArray("pin");
    result["order"] = 0;
    result["rev"] = false;
    result["skip"] = 0;
    result["ref"] = false;
    result["rgbwm"] = 0;
    return result;
}

JsonObject trophyLedStripe(JsonArray outputs, int start, int len, int dataPin, int clockPin) {
    auto result = trophyBaseOutput(outputs);
    result["type"] = 51; // HD107S-2020 is compatible to the APA102 type
    result["start"] = start;
    result["len"] = len;
    result["pin"].add(dataPin);
    result["pin"].add(clockPin);
    result["freq"] = 5000; // "Normal"
    return result;
}

JsonObject trophySinglePwm(JsonArray outputs, int start, int dataPin) {
    auto result = trophyBaseOutput(outputs);
    result["type"] = 41; // "white pwm"
    result["start"] = start;
    result["len"] = 1;
    result["pin"].add(dataPin);
    result["freq"] = 6510;
    return result;
}
