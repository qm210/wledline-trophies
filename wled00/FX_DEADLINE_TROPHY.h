#pragma once

#include "FX.h"
#include "../usermods/DEADLINE_TROPHY/DeadlineTrophy.h"

extern uint16_t mode_static(void);

//////////////////////////////////////////////////////////
//  Deadline Trophy FX
//////////////////////////////////////////////////////////
// There's a lot of useful stuff in
// FX.cpp -> see the DEV INFO, also, other FX for "inspiration" :)
// wled_math.cpp
// util.cpp
// -- you can also reinvent the wheel.
// I have never tested how efficient we really _HAVE_ to be,
// thus all the helper functions. ..qm
//////////////////////////////////////////////////////////
//
// Notes:
// - the if (...SEGMENT) { ... } is due to "this FX will be evaluated for every segment on its own"
//   remember, that you also have to set it for each segment to be actually active.
// - for "state" variables, there are two uint8_t SEGMENT.aux0 and SEGMENT.aux1,
//   but you can also declare any variable "static" to be more flexible (QM figures that makes sense here)
// - the EVERY_NTH_CALL(n) { ... } helper is great for debugging.

static CHSV hsvNone = CHSV(0, 0, 0);

float elapsed = 0.;
float deltaBeat = 0.;
float beat = 0.;
float globalTime = 0.;
float bpm = 0.;

uint16_t mode_DeadlineTrophy(void) {
    using namespace DeadlineTrophy;
    using namespace DeadlineTrophy::FxHelpers;
    CRGB segcolor0 = CRGB(SEGCOLOR(0));
    CHSV color = rgb2hsv_approximate(segcolor0);
    CHSV color_(color);

    SEGMENT.fill(BLACK);

    return FRAMETIME;
}

static const char _data_FX_MODE_DEADLINE_TROPHY[] PROGMEM =
    "DEADLINE Trophy@BPM,!,Contour Intensity,!,!;!,!;!;2;c1=0,sx=110,pal=59";
// <-- cf. https://kno.wled.ge/interfaces/json-api/#effect-metadata
// <EffectParameters>;<Colors>;<Palette>;<Flags>;<Defaults>
// <Colors> = !,! = Two Defaults
// <Palette> = ! = Default enabled (Empty would disable palette selection)
// <Flags> = 2 for "it needs the 2D matrix", add "v" and/or "f" for AudioReactive volume and frequency.
// <Defaults>: <ParameterCode>=<Value>
//             si=0 is "sound interaction" (for AudioReactive),
// <EffectParameters>: These are then read by (if defined, comma-separated)
//   sx = SEGMENT.speed (int 0-255)
//   ix = SEGMENT.intensity (int 0-255)
//   c1 = SEGMENT.custom1 (int 0-255)
//   c2 = SEGMENT.custom2 (int 0-255)
//   c3 = SEGMENT.custom3 (int 0-31)
//   o1 = SEGMENT.check1 (bool)
//   o2 = SEGMENT.check2 (bool)
//   o3 = SEGMENT.check3 (bool)
