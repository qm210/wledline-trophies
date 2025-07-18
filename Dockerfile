# === Base Image ===
# prepares the build environment with python, platformio and node to build a .bin file.
#
# the ultimate idea is that this is called as (mounting in your modifications in data/)
#   > podman run --rm -v ${PWD}/data:/mnt <name>
#
# note: I figured I'll take a python-with-platformio base image
#       and then install node there, instead of the other way round.
#       Might think about blahblahbloobloobloooh, but didn't, so far.

FROM takigama/platformio

WORKDIR /opt

COPY . /opt

RUN apt-get update \
 && apt-get install -y curl gnupg udev \
 && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
 && apt-get install -y nodejs \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*
RUN npm install -g npm@latest

RUN chmod +x ./container_build.sh

CMD ["/bin/bash", "./container_build.sh"]
