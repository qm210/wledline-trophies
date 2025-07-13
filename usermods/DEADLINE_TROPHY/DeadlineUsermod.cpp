#include "wled.h"

#include "DeadlineUsermod.h"
#include "DeadlineTrophy.h"

static DeadlineUsermod deadlineUsermod;
REGISTER_USERMOD(deadlineUsermod);

void DeadlineUsermod::sendTrophyUdp()
{
  // Cannot start the sender in setup() because the network stuff is then not initialized yet.
  if (!udpSenderConnected) {
    if (udpSenderPort == 0) {
      return;
    }
    udpSenderConnected = udpSender.begin(udpSenderPort);

    DEBUG_PRINTF("[DEADLINE_TROPHY] UDP Sender Port: %d - Connected? %d\n",
        udpSenderPort, udpSenderConnected
    );

    if (!udpSenderConnected) {
      if (doSendUdp) {
        DEBUG_PRINTLN(F("[DEADLINE_TROPHY] Wanted to send UDP values but is not connected."));
      }
      return;
    }
  }
  else if (udpSenderPort == 0) {
    udpSender.stop();
  }

  bool doLog = doDebugLogUdp || doOneVerboseDebugLogUdp;

  if (sendUdpEverySec > 0.) {
    if (sendUdpInSec <= 0.) {
        doSendUdp = true;
        sendUdpInSec = sendUdpEverySec;
        if (doLog) {
            DEBUG_PRINTF("[DEADLINE_TROPHY-UDP] Send! Running %f sec. and next in %f sec.\n", runningSec, sendUdpInSec);
        }
    } else {
        sendUdpInSec -= elapsedSec;
        return;
    }
  }

  if (!doSendUdp) {
    return;
  }

  if (doSendUdp && !keepSendingUdp) {
    doSendUdp = false;
  }

  // Caution: Using DRGB protocol, i.e. limited to 490
  // This is enough for the DL Trophy (172 LEDs).
  // If this is decoupled and more are needed -> use DNRGB
  const int packetSize = 2 + 3 * DeadlineTrophy::N_LEDS_TOTAL;
  udpPacket[0] = 2; // protocol index (2 = DRGB, 4 = DNRGB)
  udpPacket[1] = 255; // timeout in ms = forever

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
    s = 2 + 3 * index;
    udpPacket[s++] = r;
    udpPacket[s++] = g;
    udpPacket[s++] = b;
  }
  /* for (int i=0; i < nLeds; i++) {
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = R(color);
    uint8_t g = G(color);
    uint8_t b = B(color);
    udpPacket[s++] = r;
    udpPacket[s++] = g;
    udpPacket[s++] = b;
  } */

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
