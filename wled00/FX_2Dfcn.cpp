/*
  FX_2Dfcn.cpp contains all 2D utility functions

  Copyright (c) 2022  Blaz Kristan (https://blaz.at/home)
  Licensed under the EUPL v. 1.2 or later
  Adapted from code originally licensed under the MIT license

  Parts of the code adapted from WLED Sound Reactive
*/
#include "wled.h"
#include "palettes.h"

// setUpMatrix() - constructs ledmap array from matrix of panels with WxH pixels
// this converts physical (possibly irregular) LED arrangement into well defined
// array of logical pixels: fist entry corresponds to left-topmost logical pixel
// followed by horizontal pixels, when Segment::maxWidth logical pixels are added they
// are followed by next row (down) of Segment::maxWidth pixels (and so forth)
// note: matrix may be comprised of multiple panels each with different orientation
// but ledmap takes care of that. ledmap is constructed upon initialization
// so matrix should disable regular ledmap processing
void WS2812FX::setUpMatrix() {
#ifdef USE_DEADLINE_CONFIG
  setUpDeadlineTrophy();
  return;
#endif

#ifndef WLED_DISABLE_2D
  // isMatrix is set in cfg.cpp or set.cpp
  if (isMatrix) {
    // calculate width dynamically because it may have gaps
    Segment::maxWidth = 1;
    Segment::maxHeight = 1;
    for (const Panel &p : panel) {
      if (p.xOffset + p.width > Segment::maxWidth) {
        Segment::maxWidth = p.xOffset + p.width;
      }
      if (p.yOffset + p.height > Segment::maxHeight) {
        Segment::maxHeight = p.yOffset + p.height;
      }
    }

    // safety check
    if (Segment::maxWidth * Segment::maxHeight > MAX_LEDS || Segment::maxWidth > 255 || Segment::maxHeight > 255 || Segment::maxWidth <= 1 || Segment::maxHeight <= 1) {
      DEBUG_PRINTLN(F("2D Bounds error."));
      setMatrix(false);
      Segment::maxWidth = _length;
      Segment::maxHeight = 1;
      panel.clear(); // release memory allocated by panels
      panel.shrink_to_fit(); // release memory if allocated
      resetSegments();
      return;
    }

    suspend();
    waitForIt();

    customMappingSize = 0; // prevent use of mapping if anything goes wrong

    d_free(customMappingTable);
    customMappingTable = static_cast<uint16_t*>(d_malloc(sizeof(uint16_t)*getLengthTotal())); // prefer to not use SPI RAM

    if (customMappingTable) {
      customMappingSize = getLengthTotal();

      // fill with empty in case we don't fill the entire matrix
      unsigned matrixSize = Segment::maxWidth * Segment::maxHeight;
      for (unsigned i = 0; i<matrixSize; i++) customMappingTable[i] = 0xFFFFU;
      for (unsigned i = matrixSize; i<getLengthTotal(); i++) customMappingTable[i] = i; // trailing LEDs for ledmap (after matrix) if it exist

      // we will try to load a "gap" array (a JSON file)
      // the array has to have the same amount of values as mapping array (or larger)
      // "gap" array is used while building ledmap (mapping array)
      // and discarded afterwards as it has no meaning after the process
      // content of the file is just raw JSON array in the form of [val1,val2,val3,...]
      // there are no other "key":"value" pairs in it
      // allowed values are: -1 (missing pixel/no LED attached), 0 (inactive/unused pixel), 1 (active/used pixel)
      char    fileName[32]; strcpy_P(fileName, PSTR("/2d-gaps.json"));
      bool    isFile = WLED_FS.exists(fileName);
      size_t  gapSize = 0;
      int8_t *gapTable = nullptr;

      if (isFile && requestJSONBufferLock(20)) {
        DEBUG_PRINT(F("Reading LED gap from "));
        DEBUG_PRINTLN(fileName);
        // read the array into global JSON buffer
        if (readObjectFromFile(fileName, nullptr, pDoc)) {
          // the array is similar to ledmap, except it has only 3 values:
          // -1 ... missing pixel (do not increase pixel count)
          //  0 ... inactive pixel (it does count, but should be mapped out (-1))
          //  1 ... active pixel (it will count and will be mapped)
          JsonArray map = pDoc->as<JsonArray>();
          gapSize = map.size();
          if (!map.isNull() && gapSize >= matrixSize) { // not an empty map
            gapTable = static_cast<int8_t*>(p_malloc(gapSize));
            if (gapTable) for (size_t i = 0; i < gapSize; i++) {
              gapTable[i] = constrain(map[i], -1, 1);
            }
          }
        }
        DEBUG_PRINTLN(F("Gaps loaded."));
        releaseJSONBufferLock();
      }

      unsigned x, y, pix=0; //pixel
      for (const Panel &p : panel) {
        unsigned h = p.vertical ? p.height : p.width;
        unsigned v = p.vertical ? p.width  : p.height;
        for (size_t j = 0; j < v; j++){
          for(size_t i = 0; i < h; i++) {
            y = (p.vertical?p.rightStart:p.bottomStart) ? v-j-1 : j;
            x = (p.vertical?p.bottomStart:p.rightStart) ? h-i-1 : i;
            x = p.serpentine && j%2 ? h-x-1 : x;
            size_t index = (p.yOffset + (p.vertical?x:y)) * Segment::maxWidth + p.xOffset + (p.vertical?y:x);
            if (!gapTable || (gapTable && gapTable[index] >  0)) customMappingTable[index] = pix; // a useful pixel (otherwise -1 is retained)
            if (!gapTable || (gapTable && gapTable[index] >= 0)) pix++; // not a missing pixel
          }
        }
      }

      // delete gap array as we no longer need it
      p_free(gapTable);
      resume();

      #ifdef WLED_DEBUG
      DEBUG_PRINT(F("Matrix ledmap:"));
      for (unsigned i=0; i<customMappingSize; i++) {
        if (!(i%Segment::maxWidth)) DEBUG_PRINTLN();
        DEBUG_PRINTF_P(PSTR("%4d,"), customMappingTable[i]);
      }
      DEBUG_PRINTLN();
      #endif
    } else { // memory allocation error
      DEBUG_PRINTLN(F("ERROR 2D LED map allocation error."));
      setMatrix(false);
      panel.clear();
      Segment::maxWidth = _length;
      Segment::maxHeight = 1;
      resetSegments();
    }
  }
#else
  setMatrix(false); // no matter what config says
#endif
}

#ifdef USE_DEADLINE_CONFIG
void WS2812FX::setUpDeadlineTrophy() {
    setMatrix(true);

    // the empty lines are due to the actual non-equidistant lines in the logo
    const int logoW = 27;
    const int logoH = 21;
    const int baseEdge = 18; // edge length 16 + 2 of the adjacent edges

    // current Matrix implementation is named irresponsibly, but let's live with that. for now.
    Segment::maxWidth = logoW;
    Segment::maxHeight = logoH + baseEdge;
    _mainSegment = 0; // is the logo, no idea where _mainSegment is used 'n' stuff.

    if (customMappingTable != nullptr) delete[] customMappingTable;
    customMappingSize = Segment::maxWidth * Segment::maxHeight;
    const uint16_t __ = (uint16_t)-1;
    customMappingTable = new uint16_t[customMappingSize] {
         __, __, __, 97, __, 98, __, 99, __,100, __,101, __,102, __,103, __,104, __,105, __, __, __, __, __, __, __,
         __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         __, __, 96, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         __, 95, __, 94, __, 93, __, 92, __, 91, __, 90, __, 89, __, 88, __, 87, __, 86, __, 85, __, __, __, __, __,
         __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         73, __, 74, __, 75, __, 76, __, 77, __, 78, __, 79, __, 80, __, 81, __, 82, __, 83, __, 84, __, __, __, __,
         __, 72, __, 71, __, 70, __, 69, __, 68, __, 67, __, 66, __, 65, __, 64, __, 63, __, 62, __, 61, __, __, __,
         __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         __, __,  0, __,  1, __,  2, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, 59, __, 60, __, __,
         __, __, __,  5, __,  4, __,  3, __, __, __, __, __, __, __, __, __, __, __, __, __, 58, __, 57, __, 56, __,
         __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         __, __, __, __,  6, __,  7, __,  8, __, __, __, __, __, __, __, __, __, __, __, 53, __, 54, __, 55, __, __,
         __, __, __, __, __, 11, __, 10, __,  9, __, __, __, __, __, __, __, __, __, 52, __, 51, __, 50, __, __, __,
         __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         __, __, __, __, __, __, 12, __, 13, __, 14, __, __, __, __, __, __, __, 47, __, 48, __, 49, __, __, __, __,
         __, __, __, __, __, __, __, 17, __, 16, __, 15, __, __, __, __, __, __, __, 46, __, 45, __, __, __, __, __,
         __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         __, __, __, __, __, __, __, __, 18, __, 19, __, 20, __, 21, __, 22, __, 23, __, 24, __, 25, __, 26, __, __,
         __, __, __, __, __, __, __, __, __, 35, __, 34, __, 33, __, 32, __, 31, __, 30, __, 29, __, 28, __, 27, __,
         __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
         __, __, __, __, __, __, __, __, __, __, 36, __, 37, __, 38, __, 39, __, 40, __, 41, __, 42, __, 43, __, 44,
         __,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121, __,170, __, __, __, __, __, __, __, __,
        122, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,123,171, __, __, __, __, __, __, __, __,
        124, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,125, __, __, __, __, __, __, __, __, __,
        126, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,127, __, __, __, __, __, __, __, __, __,
        128, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,129, __, __, __, __, __, __, __, __, __,
        130, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,131, __, __, __, __, __, __, __, __, __,
        132, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,133, __, __, __, __, __, __, __, __, __,
        134, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,135, __, __, __, __, __, __, __, __, __,
        136, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,137, __, __, __, __, __, __, __, __, __,
        138, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,139, __, __, __, __, __, __, __, __, __,
        140, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,141, __, __, __, __, __, __, __, __, __,
        142, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,143, __, __, __, __, __, __, __, __, __,
        144, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,145, __, __, __, __, __, __, __, __, __,
        146, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,147, __, __, __, __, __, __, __, __, __,
        148, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,149, __, __, __, __, __, __, __, __, __,
        150, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,151, __, __, __, __, __, __, __, __, __,
        152, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,153, __, __, __, __, __, __, __, __, __,
         __,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169, __, __, __, __, __, __, __, __, __, __,
    };

    _length = 172; // hard code the total number of LEDs, while we're at it

    size_t s = 4;
    const char* segName[] = {"Logo", "Base", "Back Spot", "Floor Spot"};
    _segments.clear();
    _segments.reserve(s); // prevent reallocations

    // this Matrix-Linear-Hybrid thing is not great, just shoehorn the single LEDs in there as 2D segments.
    _segments.push_back(Segment(0, logoW, 0, logoH));
    _segments.push_back(Segment(0, baseEdge, logoH, logoH + baseEdge));
    _segments.push_back(Segment(baseEdge, baseEdge + 1, logoH, logoH + 1));
    _segments.push_back(Segment(baseEdge, baseEdge + 1, logoH + 1, logoH + 2));

    for (size_t i=0; i<s; i++) {
        _segments[i].setName(segName[i]);
        _segments[i].setDeadlineCapabilities(i);
        _segments[i].options =
            i == 0
                ? SEGMENT_ON | SELECTED
                : SEGMENT_ON;
        _segments[i].mode = FX_MODE_DEADLINE_TROPHY_2024;

        // these are the three colors per segment (the palette is applied onto that, somehow)
        _segments[i].colors[0] = 0xFFFFFF;
        _segments[i].colors[1] = 0xFFFFFF;
        _segments[i].colors[2] = 0xFFFFFF;
    }
    // one could use multiple segments over the same LEDs and then blend, but... who knows
    // Segment::modeBlend(true);
}
#endif


///////////////////////////////////////////////////////////
// Segment:: routines
///////////////////////////////////////////////////////////

#ifndef WLED_DISABLE_2D
// pixel is clipped if it falls outside clipping range
// if clipping start > stop the clipping range is inverted
bool IRAM_ATTR_YN Segment::isPixelXYClipped(int x, int y) const {
  if (blendingStyle != BLEND_STYLE_FADE && isInTransition() && _clipStart != _clipStop) {
    const bool invertX = _clipStart  > _clipStop;
    const bool invertY = _clipStartY > _clipStopY;
    const int  cStartX = invertX ? _clipStop   : _clipStart;
    const int  cStopX  = invertX ? _clipStart  : _clipStop;
    const int  cStartY = invertY ? _clipStopY  : _clipStartY;
    const int  cStopY  = invertY ? _clipStartY : _clipStopY;
    if (blendingStyle == BLEND_STYLE_FAIRY_DUST) {
      const unsigned width = cStopX - cStartX;          // assumes full segment width (faster than virtualWidth())
      const unsigned len = width * (cStopY - cStartY);  // assumes full segment height (faster than virtualHeight())
      if (len < 2) return false;
      const unsigned shuffled = hashInt(x + y * width) % len;
      const unsigned pos = (shuffled * 0xFFFFU) / len;
      return progress() <= pos;
    }
    if (blendingStyle == BLEND_STYLE_CIRCULAR_IN || blendingStyle == BLEND_STYLE_CIRCULAR_OUT) {
      const int cx   = (cStopX-cStartX+1) / 2;
      const int cy   = (cStopY-cStartY+1) / 2;
      const bool out = (blendingStyle == BLEND_STYLE_CIRCULAR_OUT);
      const unsigned prog = out ? progress() : 0xFFFFU - progress();
      int radius2    = max(cx, cy) * prog / 0xFFFF;
      radius2 = 2 * radius2 * radius2;
      if (radius2 == 0) return out;
      const int dx = x - cx;
      const int dy = y - cy;
      const bool outside = dx * dx + dy * dy > radius2;
      return out ? outside : !outside;
    }
    bool xInside = (x >= cStartX && x < cStopX); if (invertX) xInside = !xInside;
    bool yInside = (y >= cStartY && y < cStopY); if (invertY) yInside = !yInside;
    const bool clip = blendingStyle == BLEND_STYLE_OUTSIDE_IN ? xInside || yInside : xInside && yInside;
    return !clip;
  }
  return false;
}

void IRAM_ATTR_YN Segment::setPixelColorXY(int x, int y, uint32_t col) const
{
  if (!isActive()) return; // not active
  if (x >= (int)vWidth() || y >= (int)vHeight() || x < 0 || y < 0) return;  // if pixel would fall out of virtual segment just exit
  setPixelColorXYRaw(x, y, col);
}

#ifdef WLED_USE_AA_PIXELS
// anti-aliased version of setPixelColorXY()
void Segment::setPixelColorXY(float x, float y, uint32_t col, bool aa) const
{
  if (!isActive()) return; // not active
  if (x<0.0f || x>1.0f || y<0.0f || y>1.0f) return; // not normalized

  float fX = x * (vWidth()-1);
  float fY = y * (vHeight()-1);
  if (aa) {
    unsigned xL = roundf(fX-0.49f);
    unsigned xR = roundf(fX+0.49f);
    unsigned yT = roundf(fY-0.49f);
    unsigned yB = roundf(fY+0.49f);
    float    dL = (fX - xL)*(fX - xL);
    float    dR = (xR - fX)*(xR - fX);
    float    dT = (fY - yT)*(fY - yT);
    float    dB = (yB - fY)*(yB - fY);
    uint32_t cXLYT = getPixelColorXY(xL, yT);
    uint32_t cXRYT = getPixelColorXY(xR, yT);
    uint32_t cXLYB = getPixelColorXY(xL, yB);
    uint32_t cXRYB = getPixelColorXY(xR, yB);

    if (xL!=xR && yT!=yB) {
      setPixelColorXY(xL, yT, color_blend(col, cXLYT, uint8_t(sqrtf(dL*dT)*255.0f))); // blend TL pixel
      setPixelColorXY(xR, yT, color_blend(col, cXRYT, uint8_t(sqrtf(dR*dT)*255.0f))); // blend TR pixel
      setPixelColorXY(xL, yB, color_blend(col, cXLYB, uint8_t(sqrtf(dL*dB)*255.0f))); // blend BL pixel
      setPixelColorXY(xR, yB, color_blend(col, cXRYB, uint8_t(sqrtf(dR*dB)*255.0f))); // blend BR pixel
    } else if (xR!=xL && yT==yB) {
      setPixelColorXY(xR, yT, color_blend(col, cXLYT, uint8_t(dL*255.0f))); // blend L pixel
      setPixelColorXY(xR, yT, color_blend(col, cXRYT, uint8_t(dR*255.0f))); // blend R pixel
    } else if (xR==xL && yT!=yB) {
      setPixelColorXY(xR, yT, color_blend(col, cXLYT, uint8_t(dT*255.0f))); // blend T pixel
      setPixelColorXY(xL, yB, color_blend(col, cXLYB, uint8_t(dB*255.0f))); // blend B pixel
    } else {
      setPixelColorXY(xL, yT, col); // exact match (x & y land on a pixel)
    }
  } else {
    setPixelColorXY(uint16_t(roundf(fX)), uint16_t(roundf(fY)), col);
  }
}
#endif

// returns RGBW values of pixel
uint32_t IRAM_ATTR_YN Segment::getPixelColorXY(int x, int y) const {
  if (!isActive()) return 0; // not active
  if (x >= (int)vWidth() || y >= (int)vHeight() || x<0 || y<0) return 0;  // if pixel would fall out of virtual segment just exit
  return getPixelColorXYRaw(x,y);
}

// 2D blurring, can be asymmetrical
void Segment::blur2D(uint8_t blur_x, uint8_t blur_y, bool smear) const {
  if (!isActive()) return; // not active
  const unsigned cols = vWidth();
  const unsigned rows = vHeight();
  const auto XY = [&](unsigned x, unsigned y){ return x + y*cols; };
  uint32_t lastnew; // not necessary to initialize lastnew and last, as both will be initialized by the first loop iteration
  uint32_t last;
  if (blur_x) {
    const uint8_t keepx = smear ? 255 : 255 - blur_x;
    const uint8_t seepx = blur_x >> 1;
    for (unsigned row = 0; row < rows; row++) { // blur rows (x direction)
      uint32_t carryover = BLACK;
      uint32_t curnew = BLACK;
      for (unsigned x = 0; x < cols; x++) {
        uint32_t cur = getPixelColorRaw(XY(x, row));
        uint32_t part = color_fade(cur, seepx);
        curnew = color_fade(cur, keepx);
        if (x > 0) {
          if (carryover) curnew = color_add(curnew, carryover);
          uint32_t prev = color_add(lastnew, part);
          // optimization: only set pixel if color has changed
          if (last != prev) setPixelColorRaw(XY(x - 1, row), prev);
        } else setPixelColorRaw(XY(x, row), curnew); // first pixel
        lastnew = curnew;
        last = cur; // save original value for comparison on next iteration
        carryover = part;
      }
      setPixelColorRaw(XY(cols-1, row), curnew); // set last pixel
    }
  }
  if (blur_y) {
    const uint8_t keepy = smear ? 255 : 255 - blur_y;
    const uint8_t seepy = blur_y >> 1;
    for (unsigned col = 0; col < cols; col++) {
      uint32_t carryover = BLACK;
      uint32_t curnew = BLACK;
      for (unsigned y = 0; y < rows; y++) {
        uint32_t cur = getPixelColorRaw(XY(col, y));
        uint32_t part = color_fade(cur, seepy);
        curnew = color_fade(cur, keepy);
        if (y > 0) {
          if (carryover) curnew = color_add(curnew, carryover);
          uint32_t prev = color_add(lastnew, part);
          // optimization: only set pixel if color has changed
          if (last != prev) setPixelColorRaw(XY(col, y - 1), prev);
        } else setPixelColorRaw(XY(col, y), curnew); // first pixel
        lastnew = curnew;
        last = cur; //save original value for comparison on next iteration
        carryover = part;
      }
      setPixelColorRaw(XY(col, rows - 1), curnew);
    }
  }
}

/*
// 2D Box blur
void Segment::box_blur(unsigned radius, bool smear) {
  if (!isActive() || radius == 0) return; // not active
  if (radius > 3) radius = 3;
  const unsigned d = (1 + 2*radius) * (1 + 2*radius); // averaging divisor
  const unsigned cols = vWidth();
  const unsigned rows = vHeight();
  uint16_t *tmpRSum = new uint16_t[cols*rows];
  uint16_t *tmpGSum = new uint16_t[cols*rows];
  uint16_t *tmpBSum = new uint16_t[cols*rows];
  uint16_t *tmpWSum = new uint16_t[cols*rows];
  // fill summed-area table (https://en.wikipedia.org/wiki/Summed-area_table)
  for (unsigned x = 0; x < cols; x++) {
    unsigned rS, gS, bS, wS;
    unsigned index;
    rS = gS = bS = wS = 0;
    for (unsigned y = 0; y < rows; y++) {
      index = x * cols + y;
      if (x > 0) {
        unsigned index2 = (x - 1) * cols + y;
        tmpRSum[index] = tmpRSum[index2];
        tmpGSum[index] = tmpGSum[index2];
        tmpBSum[index] = tmpBSum[index2];
        tmpWSum[index] = tmpWSum[index2];
      } else {
        tmpRSum[index] = 0;
        tmpGSum[index] = 0;
        tmpBSum[index] = 0;
        tmpWSum[index] = 0;
      }
      uint32_t c = getPixelColorXY(x, y);
      rS += R(c);
      gS += G(c);
      bS += B(c);
      wS += W(c);
      tmpRSum[index] += rS;
      tmpGSum[index] += gS;
      tmpBSum[index] += bS;
      tmpWSum[index] += wS;
    }
  }
  // do a box blur using pre-calculated sums
  for (unsigned x = 0; x < cols; x++) {
    for (unsigned y = 0; y < rows; y++) {
      // sum = D + A - B - C where k = (x,y)
      // +----+-+---- (x)
      // |    | |
      // +----A-B
      // |    |k|
      // +----C-D
      // |
      //(y)
      unsigned x0 = x < radius ? 0 : x - radius;
      unsigned y0 = y < radius ? 0 : y - radius;
      unsigned x1 = x >= cols - radius ? cols - 1 : x + radius;
      unsigned y1 = y >= rows - radius ? rows - 1 : y + radius;
      unsigned A = x0 * cols + y0;
      unsigned B = x1 * cols + y0;
      unsigned C = x0 * cols + y1;
      unsigned D = x1 * cols + y1;
      unsigned r = tmpRSum[D] + tmpRSum[A] - tmpRSum[C] - tmpRSum[B];
      unsigned g = tmpGSum[D] + tmpGSum[A] - tmpGSum[C] - tmpGSum[B];
      unsigned b = tmpBSum[D] + tmpBSum[A] - tmpBSum[C] - tmpBSum[B];
      unsigned w = tmpWSum[D] + tmpWSum[A] - tmpWSum[C] - tmpWSum[B];
      setPixelColorXY(x, y, RGBW32(r/d, g/d, b/d, w/d));
    }
  }
  delete[] tmpRSum;
  delete[] tmpGSum;
  delete[] tmpBSum;
  delete[] tmpWSum;
}
*/
void Segment::moveX(int delta, bool wrap) const {
  if (!isActive() || !delta) return; // not active
  const int vW = vWidth();   // segment width in logical pixels (can be 0 if segment is inactive)
  const int vH = vHeight();  // segment height in logical pixels (is always >= 1)
  const auto XY = [&](unsigned x, unsigned y){ return x + y*vW; };
  int absDelta = abs(delta);
  if (absDelta >= vW) return;
  uint32_t newPxCol[vW];
  int newDelta;
  int stop = vW;
  int start = 0;
  if (wrap) newDelta = (delta + vW) % vW; // +cols in case delta < 0
  else {
    if (delta < 0) start = absDelta;
    stop = vW - absDelta;
    newDelta = delta > 0 ? delta : 0;
  }
  for (int y = 0; y < vH; y++) {
    for (int x = 0; x < stop; x++) {
      int srcX = x + newDelta;
      if (wrap) srcX %= vW; // Wrap using modulo when `wrap` is true
      newPxCol[x] = getPixelColorRaw(XY(srcX, y));
    }
    for (int x = 0; x < stop; x++) setPixelColorRaw(XY(x + start, y), newPxCol[x]);
  }
}

void Segment::moveY(int delta, bool wrap) const {
  if (!isActive() || !delta) return; // not active
  const int vW = vWidth();   // segment width in logical pixels (can be 0 if segment is inactive)
  const int vH = vHeight();  // segment height in logical pixels (is always >= 1)
  const auto XY = [&](unsigned x, unsigned y){ return x + y*vW; };
  int absDelta = abs(delta);
  if (absDelta >= vH) return;
  uint32_t newPxCol[vH];
  int newDelta;
  int stop = vH;
  int start = 0;
  if (wrap) newDelta = (delta + vH) % vH; // +rows in case delta < 0
  else {
    if (delta < 0) start = absDelta;
    stop = vH - absDelta;
    newDelta = delta > 0 ? delta : 0;
  }
  for (int x = 0; x < vW; x++) {
    for (int y = 0; y < stop; y++) {
      int srcY = y + newDelta;
      if (wrap) srcY %= vH; // Wrap using modulo when `wrap` is true
      newPxCol[y] = getPixelColorRaw(XY(x, srcY));
    }
    for (int y = 0; y < stop; y++) setPixelColorRaw(XY(x, y + start), newPxCol[y]);
  }
}

// move() - move all pixels in desired direction delta number of pixels
// @param dir direction: 0=left, 1=left-up, 2=up, 3=right-up, 4=right, 5=right-down, 6=down, 7=left-down
// @param delta number of pixels to move
// @param wrap around
void Segment::move(unsigned dir, unsigned delta, bool wrap) const {
  if (delta==0) return;
  switch (dir) {
    case 0: moveX( delta, wrap);                      break;
    case 1: moveX( delta, wrap); moveY( delta, wrap); break;
    case 2:                      moveY( delta, wrap); break;
    case 3: moveX(-delta, wrap); moveY( delta, wrap); break;
    case 4: moveX(-delta, wrap);                      break;
    case 5: moveX(-delta, wrap); moveY(-delta, wrap); break;
    case 6:                      moveY(-delta, wrap); break;
    case 7: moveX( delta, wrap); moveY(-delta, wrap); break;
  }
}

void Segment::drawCircle(uint16_t cx, uint16_t cy, uint8_t radius, uint32_t col, bool soft) const {
  if (!isActive() || radius == 0) return; // not active
  if (soft) {
    // Xiaolin Wu’s algorithm
    const int rsq = radius*radius;
    int x = 0;
    int y = radius;
    unsigned oldFade = 0;
    while (x < y) {
      float yf = sqrtf(float(rsq - x*x)); // needs to be floating point
      uint8_t fade = float(0xFF) * (ceilf(yf) - yf); // how much color to keep
      if (oldFade > fade) y--;
      oldFade = fade;
      int px, py;
      for (uint8_t i = 0; i < 16; i++) {
          int swaps = (i & 0x4 ? 1 : 0); // 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1
          int adj =  (i < 8) ? 0 : 1;    // 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1
          int dx = (i & 1) ? -1 : 1;     // 1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1
          int dy = (i & 2) ? -1 : 1;     // 1,  1, -1, -1,  1,  1, -1, -1,  1,  1, -1, -1,  1,  1, -1, -1
          if (swaps) {
              px = cx + (y - adj) * dx;
              py = cy + x * dy;
          } else {
              px = cx + x * dx;
              py = cy + (y - adj) * dy;
          }
          uint32_t pixCol = getPixelColorXY(px, py);
          setPixelColorXY(px, py, adj ?
              color_blend(pixCol, col, fade) :
              color_blend(col, pixCol, fade));
      }
      x++;
    }
  } else {
    // Bresenham’s Algorithm
    int d = 3 - (2*radius);
    int y = radius, x = 0;
    while (y >= x) {
    for (int i = 0; i < 4; i++) {
        int dx = (i & 1) ? -x : x;
        int dy = (i & 2) ? -y : y;
        setPixelColorXY(cx + dx, cy + dy, col);
        setPixelColorXY(cx + dy, cy + dx, col);
    }
      x++;
      if (d > 0) {
        y--;
        d += 4 * (x - y) + 10;
      } else {
        d += 4 * x + 6;
      }
    }
  }
}

// by stepko, taken from https://editor.soulmatelights.com/gallery/573-blobs
void Segment::fillCircle(uint16_t cx, uint16_t cy, uint8_t radius, uint32_t col, bool soft) const {
  if (!isActive() || radius == 0) return; // not active
  const int vW = vWidth();   // segment width in logical pixels (can be 0 if segment is inactive)
  const int vH = vHeight();  // segment height in logical pixels (is always >= 1)
  // draw soft bounding circle
  if (soft) drawCircle(cx, cy, radius, col, soft);
  // fill it
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius &&
          int(cx)+x >= 0 && int(cy)+y >= 0 &&
          int(cx)+x < vW && int(cy)+y < vH)
        setPixelColorXY(cx + x, cy + y, col);
    }
  }
}

//line function
void Segment::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t c, bool soft) const {
  if (!isActive()) return; // not active
  const int vW = vWidth();   // segment width in logical pixels (can be 0 if segment is inactive)
  const int vH = vHeight();  // segment height in logical pixels (is always >= 1)
  if (x0 >= vW || x1 >= vW || y0 >= vH || y1 >= vH) return;

  const int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1; // x distance & step
  const int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; // y distance & step

  // single pixel (line length == 0)
  if (dx+dy == 0) {
    setPixelColorXY(x0, y0, c);
    return;
  }

  if (soft) {
    // Xiaolin Wu’s algorithm
    const bool steep = dy > dx;
    if (steep) {
      // we need to go along longest dimension
      std::swap(x0,y0);
      std::swap(x1,y1);
    }
    if (x0 > x1) {
      // we need to go in increasing fashion
      std::swap(x0,x1);
      std::swap(y0,y1);
    }
    float gradient = x1-x0 == 0 ? 1.0f : float(y1-y0) / float(x1-x0);
    float intersectY = y0;
    for (int x = x0; x <= x1; x++) {
      uint8_t keep = float(0xFF) * (intersectY-int(intersectY)); // how much color to keep
      uint8_t seep = 0xFF - keep; // how much background to keep
      int y = int(intersectY);
      if (steep) std::swap(x,y);  // temporaryly swap if steep
      // pixel coverage is determined by fractional part of y co-ordinate
      blendPixelColorXY(x, y, c, seep);
      blendPixelColorXY(x+int(steep), y+int(!steep), c, keep);
      intersectY += gradient;
      if (steep) std::swap(x,y);  // restore if steep
    }
  } else {
    // Bresenham's algorithm
    int err = (dx>dy ? dx : -dy)/2;   // error direction
    for (;;) {
      setPixelColorXY(x0, y0, c);
      if (x0==x1 && y0==y1) break;
      int e2 = err;
      if (e2 >-dx) { err -= dy; x0 += sx; }
      if (e2 < dy) { err += dx; y0 += sy; }
    }
  }
}

#include "src/font/console_font_4x6.h"
#include "src/font/console_font_5x8.h"
#include "src/font/console_font_5x12.h"
#include "src/font/console_font_6x8.h"
#include "src/font/console_font_7x9.h"

// draws a raster font character on canvas
// only supports: 4x6=24, 5x8=40, 5x12=60, 6x8=48 and 7x9=63 fonts ATM
void Segment::drawCharacter(unsigned char chr, int16_t x, int16_t y, uint8_t w, uint8_t h, uint32_t color, uint32_t col2, int8_t rotate) const {
  if (!isActive()) return; // not active
  if (chr < 32 || chr > 126) return; // only ASCII 32-126 supported
  chr -= 32; // align with font table entries
  const int font = w*h;

  // if col2 == BLACK then use currently selected palette for gradient otherwise create gradient from color and col2
  CRGBPalette16 grad = col2 ? CRGBPalette16(CRGB(color), CRGB(col2)) : SEGPALETTE; // selected palette as gradient

  for (int i = 0; i<h; i++) { // character height
    uint8_t bits = 0;
    switch (font) {
      case 24: bits = pgm_read_byte_near(&console_font_4x6[(chr * h) + i]); break;  // 4x6 font
      case 40: bits = pgm_read_byte_near(&console_font_5x8[(chr * h) + i]); break;  // 5x8 font
      case 48: bits = pgm_read_byte_near(&console_font_6x8[(chr * h) + i]); break;  // 6x8 font
      case 63: bits = pgm_read_byte_near(&console_font_7x9[(chr * h) + i]); break;  // 7x9 font
      case 60: bits = pgm_read_byte_near(&console_font_5x12[(chr * h) + i]); break; // 5x12 font
      default: return;
    }
    CRGBW c = ColorFromPalette(grad, (i+1)*255/h, 255, LINEARBLEND_NOWRAP); // NOBLEND is faster
    for (int j = 0; j<w; j++) { // character width
      int x0, y0;
      switch (rotate) {
        case -1: x0 = x + (h-1) - i; y0 = y + (w-1) - j; break; // -90 deg
        case -2:
        case  2: x0 = x + j;         y0 = y + (h-1) - i; break; // 180 deg
        case  1: x0 = x + i;         y0 = y + j;         break; // +90 deg
        default: x0 = x + (w-1) - j; y0 = y + i;         break; // no rotation
      }
      if (x0 < 0 || x0 >= (int)vWidth() || y0 < 0 || y0 >= (int)vHeight()) continue; // drawing off-screen
      if (((bits>>(j+(8-w))) & 0x01)) { // bit set
        setPixelColorXYRaw(x0, y0, c.color32);
      }
    }
  }
}

#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
void Segment::wu_pixel(uint32_t x, uint32_t y, CRGB c) const {      //awesome wu_pixel procedure by reddit u/sutaburosu
  if (!isActive()) return; // not active
  // extract the fractional parts and derive their inverses
  unsigned xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (int i = 0; i < 4; i++) {
    int wu_x = (x >> 8) + (i & 1);        // precalculate x
    int wu_y = (y >> 8) + ((i >> 1) & 1); // precalculate y
    CRGB led = getPixelColorXY(wu_x, wu_y);
    CRGB oldLed = led;
    led.r = qadd8(led.r, c.r * wu[i] >> 8);
    led.g = qadd8(led.g, c.g * wu[i] >> 8);
    led.b = qadd8(led.b, c.b * wu[i] >> 8);
    if (led != oldLed) setPixelColorXY(wu_x, wu_y, led); // don't repaint if same color
  }
}
#undef WU_WEIGHT

#endif // WLED_DISABLE_2D
