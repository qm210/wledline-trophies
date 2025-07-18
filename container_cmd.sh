#!/bin/bash

echo "Hello, Stuff in /mnt:"
echo "====="
ls /mnt
echo "====="

# if stuff is there, copy it
[ -f /mnt/my_config.h ] && cp -v /mnt/config.h wled00/
[ -f /mnt/FX_DEADLINE_TROPHY.h ] && cp -v /mnt/FX_DEADLINE_TROPHY.h wled00/

# fallback
touch wled00/my_config.h

# now let's platformio do its magic
platformio run --target upload
