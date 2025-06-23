byte udpPacket[1024];
unsigned long loggedAt = 0;

inline void sendTrophyUdp()
{
  // QM-WIP: Send all RGB info per UDP for the Trophy Simulator
  // TODO: make this universal, not DL-trophy-specfici

  if (!udpSenderConnected) {
    return;
  }

  auto now = millis();
  bool logDebug = loggedAt == 0; // || (now - loggedAt) > 20000;

  int nLeds = strip.getLengthTotal();
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

    if (logDebug) {
        DEBUG_PRINTF("[DEBUG UDP] Pixel %d = %d (%u, %u, %u)\n", i, color, r, g, b);
    }
  }

  IPAddress broadcastIp;
  broadcastIp = ~uint32_t(Network.subnetMask()) | uint32_t(Network.gatewayIP());
  senderUdp.beginPacket(broadcastIp, udpSenderPort);
  senderUdp.write(udpPacket, packetSize);
  senderUdp.endPacket();

  if (logDebug) {
    DEBUG_PRINTF("[SENT UDP PACKAGE] Size %d\n", packetSize);
    loggedAt = now;
    logDebug = false;
  }
}
