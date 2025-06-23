#include "wled.h"
/*
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/wled-dev/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 *
 * Consider the v2 usermod API if you need a more advanced feature set!
 */

#ifdef USERMOD_DEADLINE_TROPHY
  #include "../usermods/DEADLINE_TROPHY/udp_sending.h"
  // we put the following here because that is the easiest for now.
  #include "../usermods/DEADLINE_TROPHY/DeadlineTrophy.h"
  static DeadlineTrophyUsermod deadlineUsermod;
  REGISTER_USERMOD(deadlineUsermod);
#endif

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{

}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{

}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
    #ifdef USERMOD_DEADLINE_TROPHY
        sendTrophyUdp();
    #endif
}
