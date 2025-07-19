#!/bin/bash

# idea is to call this with some specific files to overwrite in /mnt
# this will then produce a .bin to upload to your controller.

# if stuff is there, copy it
[ -f /mnt/my_config.h ] && cp -v /mnt/my_config.h wled00/
[ -f /mnt/FX_DEADLINE_TROPHY.h ] && cp -v /mnt/FX_DEADLINE_TROPHY.h wled00/

# fallback
touch wled00/my_config.h

# clean output
rm -f /mnt/firmware.bin

# now let's platformio do its magic.
platformio run

# and publish the result
cp -v .pio/build/deadline_trophy/firmware.bin /mnt
# and one with clear specification for the record:
cp -v .pio/build/deadline_trophy/firmware.bin /mnt/deadline_trophy_$(date +%Y%m%d_%H%M).bin

# Maybe Zukunftsmusik: Manage the upload to the COM port from the container, like
#   platformio run --target upload
# but this needs permission to access the COM ports, blahblah, so yeah, not today.
