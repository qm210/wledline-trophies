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

So then you can run this WLED fork on the controller without any LEDs attached and visualize the


## Building via PlatformIO
Take PlatformIO (i.e. [PlatformIO for VSCode](https://platformio.org/install/ide?install=vscode)), download the repo, connect the controller via COM port, upload the shit via PlatformIO and tell me where it breaks.

## Building via our docker image
Disclaimer: If you hate using such build images, for some irrational reason or another, just use PlatformIO with the raw sources on your own
 * If you fail, just ask me
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

then mount these into the container (depending on your environment) and run the thing;
```
# Linux
podman run --rm -v $(pwd)/data:/mnt trophy-builder
# Windows PowerShell
podman run --rm -v ${PWD}/data:/mnt trophy-builder
# Windows Command Prompt
podman run --rm -v %cd%\data:/mnt trophy-builder
```
This will then run platformio including the upload.
 * make sure your ESP32 controller is connected to some available COM port.
 * also make sure the cable is not a charge-only cable ;)

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

