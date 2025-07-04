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

    if (s > 1023) {
        // see above, we are currently limited
        break;
    }

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

  // QM-TODO: sending continuously seems to "choke" the ESP32, see below - NEEDS DEBUGGING

  udpSender.beginPacket(udpSenderIp, udpSenderPort);
  udpSender.write(udpPacket, packetSize);
  udpSender.endPacket();

  if (doDebugLogUdp ) {
    DEBUG_PRINTF("[SENT UDP PACKAGE] Size %d\n", packetSize);
  }

  doOneVerboseDebugLogUdp = false;
}

/*
 * The delayed UDP packet reception you’re observing when sending large packets (around 600 bytes) from an ESP32 (WLED, WiFiUDP) is a known issue in the ESP32 ecosystem. Several factors can contribute to this behavior:

WiFi Stack Buffering and Fragmentation:
UDP packets larger than the network MTU (typically 1500 bytes for Ethernet, but lower for WiFi) may get fragmented. Even at 600 bytes, the ESP32 WiFi stack may buffer or cluster packets, leading to multiple packets being sent in bursts after a delay instead of immediately. This "lumping" behavior has been observed by others, with delays often in the hundreds of milliseconds or more.

WiFi Environment and Noise:
If the WiFi environment is noisy or crowded, packet transmission and reception can be delayed due to retransmissions, interference, or contention for airtime. Even with the ESP32 near the access point, users have reported significant delays and packet loss.

ESP32 WiFi Stack Limitations:
The ESP32’s WiFi/UDP implementation is known to have issues with high-frequency or large-payload UDP transmissions, leading to dropped or delayed packets. Some users note that the ESP32 can "choke," causing clusters of packets to arrive together after a pause, even when the sender is transmitting at regular intervals.

Power Management/Sleep:
If WiFi sleep is enabled, it can introduce additional latency. Disabling sleep (WiFi.setSleep(false);) can help, but may not eliminate the issue entirely.

UDP is Unreliable by Design:
UDP does not guarantee delivery or timing. The ESP32's implementation can exacerbate this unreliability under certain conditions, especially with larger packets or high send rates.

In summary:
This is a well-documented limitation of the ESP32’s WiFi/UDP stack, especially with larger or frequent packets. Packets may be buffered and sent in clusters, causing receivers to see bursts of data after noticeable delays. Network conditions, ESP32 firmware, and WiFi configuration can all influence the severity of this effect.
 */
