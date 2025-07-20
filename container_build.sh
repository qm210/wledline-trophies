#!/bin/bash

# idea is to call this with some specific files to overwrite in /mnt
# this will then produce a .bin to upload to your controller.

# if stuff is there, copy it
[ -f /mnt/my_config.h ] && cp -v /mnt/my_config.h wled00/
[ -f /mnt/FX_DEADLINE_TROPHY.h ] && cp -v /mnt/FX_DEADLINE_TROPHY.h wled00/

# clean output
rm -f /mnt/firmware.bin

echo
echo "\n== LET'S BUILD!! == exclamation marks."
echo

# now let's platformio do its magic.
if ! platformio run; then
    echo "== BUILD FAILED == Exclamation Mark! You win some, you lose some."
    exit $?
fi

# and publish the result
cp -v .pio/build/deadline_trophy/firmware.bin /mnt
# and one with clear specification for the record:
cp -v .pio/build/deadline_trophy/firmware.bin /mnt/deadline_trophy_$(date +%Y%m%d_%H%M).bin
# also, the partitions are useful for flashing a pristine controller
cp -v .pio/build/deadline_trophy/partitions.bin /mnt
