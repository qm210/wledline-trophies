## What this is about ##

The Trophies for the 10-year-anniversary Deadline 2024 compoparties have been absolutely overengineered but to honor all the hours spent by Thorin, QM, *etc.blalalala.$insertNameHere*, there had to be a possibility for you all to have some fun with it.

Which is why this firmware [based on WLED](https://github.com/wled) has been modified to be somewhat moddable without diving *completely* into WLED sourcecode, and to preview the effects on the trophy LEDs without needing to having ~~manipulated the crowd~~ virtuously won one of these trophies in the past yourself - you just need a suitable ESP32 (WROOM 32D) controller.

See also https://www.demoparty.berlin/deadline-trophy-compo/

* Please know that if you have any trouble understanding the following, there ARE more details [in the official README](https://github.com/qm210/wledline-trophies)
* If that doesn't help, you should manage to contact QM somehow (ask around) and he'll try to explain / fix stuff.

## Get it up and running ##

... to get the current firmware onto your ESP32:

1. You clone this repository in its most current state

2. You get yourself [PlatformIO for VS Code](https://platformio.org/install/ide?install=vscode)
   * it _can_ be done [otherwise](https://github.com/qm210/wledline-trophies#build-via-container), but for most cases just use VS Code.

3. With the controller connected via USB, find the `→ PlatformIO: Upload` (e.g. in the status bar). Do it.
   * this can take some time (~2 minutes for me), but produce no errors (warnings might happen)
   * If after a while you do not see `Writing at 0x00010000... (1 %)`, there's something wrong. Check [README: Upload Fails](https://github.com/qm210/wledline-trophies#upload-fails)

4. To see that it works, you can look for the Web UI.
   * In your local WiFis, you should see a "DL-TROPHY-AP" (access point spawned by the controller)
   * Connect to that with default password "deadline"
   * If connected, direct your browser to `http://4.3.2.1`
     * From there, you should be able to configure your local network so you don't have to do the AP thing every time
     * When you finally reach the UI front page, Check the `☆ Peek` panel. Seein' something good?
   * If that is not straightforward, see [README: Getting to work](https://github.com/qm210/wledline-trophies#getting-to-work)

   * Your local network can also be configured before compiling by setting the flags in `./wled00/my_config.h` as e.g.
   ```
   #define CLIENT_SSID "Superfunny and Innovative Network Name"
   #define CLIENT_PASS "test1234alphabadboy"
   ```

## Connecting the Simulator ##

The Peek Preview may be nice, but do you know about the Simulator?
 * https://github.com/qm210/dltrophy-simulator

This thing is a bit full of unnecessary parameters (because for QM, there's no such thing as a "main quest"), but basically,
our firmware mod sends out all the LED's RGB data (via UDP) and the Simulator listens to such messages.

For that, the firmware must know where the Simulator is listening:
1. Go to the Web UI
2. In the top menu, go `⚙ Config` > `Usermods` > scroll down to `DeadlineTrophy` > `Simulator`.
3. Enter your local machine's IP, you likely can keep the port at 3413 (unless it's taken, but qm doesn't know of a common thing that uses it)
4. Save.

## Modifying the Deadline Trophy FX ##

Now locate the `./wled00/FX_DEADLINE_TROPHY.h`.
* The `mode_DeadlineTrophy()` is what you want to change (unless you're weird).
  * so → **modify this and upload again.**
    * must be valid C++, sadly
    * the PlatformIO toolchain should support C++14, for finer details search the web for "Xtensa ESP32 toolchain" or ""xtensa-esp32-elf-gcc"
  * the existing `FX_DEADLINE_TROPHY.h` has some examples of how the trophy can be addressed,
    * but just play around with whatever you can imagine
    * there are useful tools to be found in `util.cpp`, `wled_math.cpp`
    * and of course `FX.cpp` where all the stock WLED effects live
      * look at https://kno.wled.ge/features/effects/ for inspiration
    * there's also all the trophy-specific stuff in `./wled00/usermods/DEADLINE_TROPHY/` for you to use.
* Questions? just go find a very-QM-looking entity and he'll probably help  ╮(◑‿◑)╭

Damage to the Trophy hardware can only occur for overly bright LEDs over a period of time, but this is managed by limiting the LED currents outside of what the FX function is for. You can, of course, change any other file and use that with your hardware, but for the compo, we will not run that thing, we will want a `FX_DEADLINE_TROPHY.h`.

## What else do you need to know? ##
> This part might contain the pre-existing functions / useful tools / good-to-know implementation details
> to help you make great Trophy FX, but can only do so when we know what you actually need to know.
>
>  For now, the stuff that is used in the existing `FX_DEADLINE_TROPHY.h` can be considered an okayish starting point. Have fun! Ꮷ( ＾◡＾)っ

