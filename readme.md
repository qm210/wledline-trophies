# This is QM's WLED fork for best support of the Deadline'24 Trophy ever
.. and there's oh-so-little one can do about it.

Why fork, you might ask? Because multiple reasons:
 * Hard limit of the maximum current
   * Due to a lot of LEDs tightly packed within epoxy, these can get too hot otherwise and kill the circuit
   * these CAN NOT be fixed!
 * Fixed LED segments for base, logo, and the single-white LEDs at the floor and back side
 * ability to constantly send the current RGB values via UDP
   * in order to work with my Trophy Smiuluator at https://github.com/qm210/dltrophy-simulator
 * easier development using a build image (see below)

Also, if these don't convince you, well – fork you ¯\\\_(ツ)\_/¯

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
This will then run platformio and produce a `data/firmware.bin` (also one called `deadline_trophy_<Timestamp>.bin`, just in case you tend to get distracted...).

## Upload the .bin to the Controller
One should be able to access the UI

The resulting `firmware.bin` can be uploaded via WLED's integrated Over-The-Air Updater, via
* Web UI -> Config -> Security & Updates -> Manual OTA Update
  * e.g. [http://<WLED host>/update](http://deadline.local/update) (...unless you changed that hostname.)
* Select the bin file and **Update!**

There are manual ways of doing this with `esptool` etc. but this required advanced knowledge about the partitioning,
i.e. I can prepare that documentation but if no one ever needs it, won't waste my time now... gotta do important... shrooms, or something.

This fork uses `deadline.local` as default DNS name, i.e. you can then access the UI under http://deadline.local, when in your configured network. You can also pass

### Compiler Flags
There are a lot, but these are most relevant to know
```
// local network access
#define CLIENT_SSID "guess what"
#define CLIENT_PASS "won't tell you"

// to access the UI under http://whatever.local
#define MDNS_NAME whatever

// for the Simulator
#define DEADLINE_UDP_SENDER_IP "192.168.178.20"
#define DEADLINE_UDP_SENDER_PORT 3413

// if set, this will always delete the config file on the controller at boot
#define RESET_CONFIG

// if set, this will increase logging to read from any Serial monitor (baud 115200)
#define WLED_DEBUG
```

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

