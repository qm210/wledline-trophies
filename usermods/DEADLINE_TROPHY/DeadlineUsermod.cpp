#include "wled.h"

#include "DeadlineUsermod.h"
#include "DeadlineTrophy.h"

static DeadlineUsermod deadlineUsermod;
REGISTER_USERMOD(deadlineUsermod);


void DeadlineUsermod::sendTrophyUdp()
{
  // Cannot start the sender in setup() because the network stuff is then not initialized yet.
  if (!udpSenderConnected) {
    if (udpSenderPort > 0) {
      udpSenderConnected = udpSender.begin(udpSenderPort);

      DEBUG_PRINTF("[DEADLINE_TROPHY] UDP Target: %s:%d) - Connected? %d\n",
        udpSenderIp.toString().c_str(), udpSenderPort, udpSenderConnected
      );
    }
    if (!udpSenderConnected) {
      if (doSendUdp) {
        DEBUG_PRINTLN(F("[DEADLINE_TROPHY] Wanted to send UDP values but is not connected."));
      }
      return;
    }
  }

  if (sendUdpEverySec > 0.) {
    if (sendUdpInSec <= 0.) {
        doSendUdp = true;
        sendUdpInSec = sendUdpEverySec;
        if (doDebugLogUdp) {
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

  int nLeds = DeadlineTrophy::N_LEDS_TOTAL;

  // Caution: Using DRGB protocol, i.e. limited to 490
  // This is enough for the DL Trophy (172 LEDs).
  // If this is decoupled and more are needed -> use DNRGB
  int packetSize = 2 + 3 * nLeds;
  udpPacket[0] = 2; // protocol index (2 = DRGB, 4 = DNRGB)
  udpPacket[1] = 255; // timeout in ms = forever

  int s = 2;
  for (int i=0; i < nLeds; i++) {
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = R(color);
    uint8_t g = G(color);
    uint8_t b = B(color);
    udpPacket[s++] = r;
    udpPacket[s++] = g;
    udpPacket[s++] = b;

    if (doOneVerboseDebugLogUdp) {
        DEBUG_PRINTF("[DEBUG UDP] Pixel %d = %d (%u, %u, %u)\n", i, color, r, g, b);
    }

    if (i == 0) {
        printDebugColor |= debugColor != color;
        debugColor = color;
    }
  }

  if (printDebugColor) {
    DEBUG_PRINTF("[DEBUG UDP COLOR] is now [%d, %d, %d]\n", udpPacket[2], udpPacket[3], udpPacket[4]);
    printDebugColor = false;
  }

  udpSender.beginPacket(udpSenderIp, udpSenderPort);
  udpSender.write(udpPacket, packetSize);
  udpSender.endPacket();

  if (doDebugLogUdp ) {
    DEBUG_PRINTF("[SENT UDP PACKAGE] Size %d\n", packetSize);
  }

  doOneVerboseDebugLogUdp = false;
}
