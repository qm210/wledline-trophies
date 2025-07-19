# This is QM's WLED fork for best support of the Deadline'24 Trophy ever
Modified for multiple reasons
 * Hard limit of the maximum current
   * Due to a lot of LEDs tightly packed within epoxy, these can get too hot otherwise and kill the circuit
   * these CAN NOT be fixed!
 * Fixed LED segments for base, logo, and the single-white LEDs at the floor and back side
 * ability to constantly send the current RGB values via UDP
   * in order to work with my Trophy Smiuluator at https://github.com/qm210/dltrophy-simulator
 * easier development using a build image (see below)

## If you are one of these losers who do not have a Trophy yet
...good news that I had several ounces of boque to invest into the [Simulator](https://github.com/qm210/dltrophy-simulator)

You can use this
* as long as you have a single ESP32 on your own
  * should work for a ESP32-WROOM-32D with at least 4 MB Flash
  * but try whatevery you have and tell me about it
* and as long as you get the Simulator built
  * so far, only tested on a Windows 11, but this will expand to whatever we need

So then you can run this WLED fork on the controller without any LEDs attached and visualize the theoretical output.

# Build & Upload via PlatformIO
If you hate using such build containers (because you feel irrationally smart or something) or have PlatformIO anyway:

Take PlatformIO (i.e. [PlatformIO for VSCode](https://platformio.org/install/ide?install=vscode)), download the repo, connect the controller via COM port, upload the shit via PlatformIO and tell me where it breaks.

#### Stuff to Change
Obviously, no one can stop you from changing whatever you find here, but if you just want to develop a pattern of your own, look for
* `wled00/my_config.h`
  * for some overall define flags (it should contain some documentation, speaking as of now)
* `wled00/FX_DEADLINE_TROPHY.h`
  * contains the function that is called for every time step and segment
  * modify this to make your own pattern instead
  * for inspiration, you can look to the [existing patterns in FX.cpp](wled00/FX.cpp)

# Build via Container
Less requirements on your local drive, and less pollution, and maybe less confusing – just as an option.
 * If you fail, just ask me (the qm dude)
   * If you are half as insufferable as your hate against images suggests, I will be glad to help :)
   * If you fall into the wrong half - I don't give a shit.

Now:
Can be built using Docker or the more lightweight [Podman](https://podman.io/docs/installation), as
```
podman build -t trophy-builder .
```
Then have your modifications in the `data/` folder:
 * `data/FX_DEADLINE_TROPHY.h` - your pattern (there will be some documentation somewhere on what to actually do)
 * `data/my_config.h` - your global config flags via `#define`, e.g. network settings blablahblooblooh

The `my_config.h` should at least contain the credentials for your WiFi in order to access the UI on http://deadline.local (default, configurable later on)
```
#define CLIENT_SSID "..."
#define CLIENT_PASS "..."
```

then mount these into the container (depending on your environment) and run the thing;
```
# Linux
podman run --rm -v $(pwd)/data:/mnt trophy-builder
# Windows PowerShell
podman run --rm -v ${PWD}/data:/mnt trophy-builder
# Windows Command Prompt
podman run --rm -v %cd%\data:/mnt trophy-builder
```
This will then run platformio and produce a `data/deadline_trophy.bin` (also one with a timestamp, just in case...).

### Upload the .bin to the Controller
You can then run [esptool](https://docs.espressif.com/projects/esptool/en/latest/esp32/esptool/flashing-firmware.html) to upload
```
esptool --port <Port> write-flash -z 0x10000 deadline_trophy.bin
# ... so use the correct port, looking like "/dev/ttyUSB0" under Linux and "COM5" under Windows

# Note, if esptool is not available, but pip is, just use one of these
#   pip install esptool
#   python -m pip install esptool
# ... or pip3 or whatever... guess you're old enough.
```
 * make sure your ESP32 controller is connected to some available COM port.
 * also make sure the cable is not a charge-only cable ;)
 * seems there is a `--chip esp32` that can be left out, but if something doesn't work, try the esptool command with that.
 * The `0x10000` is where the application partition should start - worked on QM's machine - tell me if it doesn't.

If no errors appeared until there, one should be able to access the UI in the configured network under http://deadline.local

### Summary: Developer Workflow
So, ideally, for the time being, you do (in this project root folder)
 * Set Up (once or seldom)
   * `podman build -t trophy-builder .`
   * adjust `data/my_config.h`
 * Then all day long (example PowerShell, adjust as required)
   * adjust `data/FX_DEADLINE_TROPHY.h` according to whatever floats your goat
   * `podman run --rm -v ${PWD}/data:/mnt trophy-builder`
   * `esptool --port COM5 write-flash -z 0x10000 .\data\firmware.bin`
 * And when the weather starts getting harsher, remember that once again, it might be Deadline soon.

## Troubleshooting

#### the Containerized Build
for troubleshooting in Podman: (if you use Docker, I assume you know your stuff anyway.)
```
# make sure the VM is actually running, otherwise call these two and build again
podman machine init
podman machine start

# might come handy:
podman images

# if there is garbage, you can remove these by
#  podman rmi <id>
#  podman image prune
```


## Forked from the offical WLED release 0.16.0
 * https://github.com/wled/WLED
 * well, forked earlier, but rebased since then.
 * Licensed under the EUPL v1.2 license
   * Credits [here](https://kno.wled.ge/about/contributors/)!

