#include "wled.h"

#include "DeadlineUsermod.h"
#include "DeadlineTrophy.h"

static DeadlineUsermod deadlineUsermod;
REGISTER_USERMOD(deadlineUsermod);

void DeadlineUsermod::readRgbValues()
{
  int s;
  // it seems that we can only use strip.getPixelColor(i), not just seg->getPixelColor or bus->getPixelColor()
  // that means, we loop over the whole matrix and skip the gaps there.
  for (size_t i=0; i < strip.getLengthTotal(); i++) {
    size_t index = strip.getMappedPixelIndex(i);
    if (index >= DeadlineTrophy::N_LEDS_TOTAL) {
        continue;
    }
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = R(color);
    uint8_t g = G(color);
    uint8_t b = B(color);
    s = 3 * index;
    rgbValues[s++] = r;
    rgbValues[s++] = g;
    rgbValues[s++] = b;
  }
}

void DeadlineUsermod::sendTrophyUdp()
{
  // Cannot start the sender in setup() because the network stuff is then not initialized yet.
  if (!udpSenderConnected) {
    if (udpSenderPort == 0) {
      return;
    }
    udpSenderConnected = udpSender.begin(udpSenderPort);

    DEBUG_PRINTF("[DEADLINE_TROPHY] UDP Sender Port: %d - Connected? %d\n", udpSenderPort, udpSenderConnected);

    if (!udpSenderConnected) {
      return;
    }
  }
  else if (udpSenderPort == 0) {
    udpSender.stop();
  }

  bool doLog = doDebugLogUdp || doOneVerboseDebugLogUdp;

  if (!udpSenderEnabled) {
    return;
  }

  // udpSenderIntervalSec must be set > 0. else the ESP32 sender queue can get clogged.
  if (sendUdpInSec > 0.) {
    sendUdpInSec -= elapsedSec;
    return;
  }
  sendUdpInSec += udpSenderIntervalSec;

  // QM Note: Using DRGB protocol, i.e. limited to 490
  // This is enough for the DL Trophy (172 LEDs).
  // If this is decoupled and more are needed -> use DNRGB
  const int packetSize = 2 + 3 * DeadlineTrophy::N_LEDS_TOTAL;
  udpPacket[0] = 2; // protocol index (2 = DRGB, 4 = DNRGB)
  udpPacket[1] = 255; // timeout in ms = forever
  memcpy(udpPacket + 2, rgbValues, DeadlineTrophy::N_LEDS_TOTAL);

  udpSender.beginPacket(udpSenderIp, udpSenderPort);
  udpSender.write(udpPacket, packetSize);
  udpSender.endPacket();

  if (doLog) {
    DEBUG_PRINTF("[SENT UDP PACKAGE] to %s:%d, size %d\n", udpSenderIp.toString().c_str(), udpSenderPort, packetSize);
  }

  if (doOneVerboseDebugLogUdp) {
    for (int i=0; i < DeadlineTrophy::N_LEDS_TOTAL; i++) {
        int s = 2 + 3*i;
        DEBUG_PRINTF("[DEBUG-UDP] index %d, color [%d, %d, %d]\n", i, udpPacket[s], udpPacket[s+1], udpPacket[s+2]);
    }
  }

  doOneVerboseDebugLogUdp = false;
}

bool DeadlineUsermod::sendLiveview(AsyncWebSocketClient *wsc)
{
    const int HEADER_VALUES = 4;
    // somewhat duplicated from ws.cpp sendLiveLedsWs(), but distinction is more manageable that way
    AsyncWebSocketBuffer wsBuf(HEADER_VALUES + N_RGB_VALUES);
    if (!wsBuf) return false; //out of memory
    uint8_t* buffer = reinterpret_cast<uint8_t*>(wsBuf.data());
    if (!buffer) return false; //out of memory

    buffer[0] = 'L';
    buffer[1] = 2;
    buffer[2] = Segment::maxWidth;
    buffer[3] = Segment::maxHeight;
    memcpy(buffer + HEADER_VALUES, rgbValues, N_RGB_VALUES);

    wsc->binary(std::move(wsBuf));
    return true;
}