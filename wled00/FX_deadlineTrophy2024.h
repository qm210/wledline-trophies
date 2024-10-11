#pragma once

#include "FX.h"

// bla bla bla quickficks
uint16_t mode_static(void);

////////////////////////////
//  Deadline Trophy 2024  //
////////////////////////////

const size_t nOuterLeft = 10;
const size_t nLeft = 35;
const size_t nBottom = 36;
const size_t nUpperRight = 17;
const size_t nOuterRight = 27;

// the bars in the Logo
const int barOuterLeft[nOuterLeft] = {
    8, 7, 6, 5, 4, 3, 2, 1, 0,
    9
};
const int barLeft[nLeft] = {
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44
};
const int barBottom[nBottom] = {
    9, 11, 30, 35, 103, 102, 97, 96, 91, 90, 85, 72, 67,
    10, 31, 34, 104, 101, 98, 95, 92, 89, 86, 71, 68,
    32, 33, 105, 100, 99, 94, 93, 88, 87, 70, 69
};
const int barUpperRight[nUpperRight] = {
    44, 46, 47, 52, 53, 58,
    45, 48, 51, 54, 57, 59,
    49, 50, 55, 56, 60
};
const int barOuterRight[nOuterRight] = {
    87, 86, 85, 84, 83, 82, 81, 80, 79,
    70, 71, 72, 73, 74, 75, 76, 77, 78,
    69, 68, 67, 66, 65, 64, 63, 62, 61
};

const size_t nBars = 5;
const int nInBar[] = {
    nOuterLeft,
    nLeft,
    nBottom,
    nUpperRight,
    nOuterRight
};
const int* indexBars[] = {
    barOuterLeft,
    barLeft,
    barBottom,
    barUpperRight,
    barOuterRight
};
const int nBase = 64;
const int indexBack = 170;
const int indexFloor = 171;

int hue[nBars] = {0};
int sat[nBars] = {0};
int val[nBars] = {0};

const int hueSpread = 4.;
const int valSpread = 2.;
const float satDecay = 0.5;
const float satSpawnChance = 0.001;


uint16_t mode_DeadlineTrophy2024(void) {
  if (!strip.isDeadlineTrophy) return mode_static();

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  size_t b = 0;
  if (SEGENV.call == 0) {
    SEGMENT.fill(BLACK);
    for (b = 0; b < nBars; b++) {
        hue[b] = random(210, 330);
        val[b] = random(0, 255);
        sat[b] = 255;
    }
  }

  size_t i;

    // LOGO
    if (strip.getCurrSegmentId() == 0) {
        for (b = 0; b < nBars; b++) {
            hue[b] = (hue[b] + random(-hueSpread, hueSpread)) % 255;
            val[b] = MAX(val[b] + random(-valSpread, valSpread), 255);
            sat[b] = static_cast<int>(satDecay * sat[b]);

            if (0.001 * random(0, 1000) < satSpawnChance) {
                sat[b] = 255;
            }

            for (i = 0; i < nInBar[i]; i++) {
                auto color = CHSV(hue[b], sat[b], val[b]);
                SEGMENT.setPixelColor(indexBars[b][i], color);
            }
        }
    }

    // BASE
    if (strip.getCurrSegmentId() == 1) {
        for (i = 0; i < nBase; i++) {
            SEGMENT.setPixelColor(i, MAGENTA);
        }
    }

    if (strip.getCurrSegmentId() == 2) {
        SEGMENT.setPixelColor(indexBack, WHITE);
    }

    if (strip.getCurrSegmentId() == 3) {
        SEGMENT.setPixelColor(indexFloor, WHITE);
    }

  return FRAMETIME;
} // mode_DeadlineTrophy2024


static const char _data_FX_MODE_DEADLINE_TROPHY_2024[] PROGMEM =
    "Deadline'24 Trophy@;;!;1";
    // <-- find out what the cryptic @... parameters mean. default is "name@;;!;1"
