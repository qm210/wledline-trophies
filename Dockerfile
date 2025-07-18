# === Base Image ===
# prepares only the build environment and all the sources
# but does not build anything (as the sources that will vary)
#
# the ultimate idea is that this is called as (mounting in your modifications in data/)
#   > podman run --rm -v ${PWD}/data:/mnt <name>

# note: I figured I'll take a python-with-platformio base image
#       and then install node there, instead of the other way round.
#		Might think about blahblahbloobloobloooh, but didn't, so far.
#
# 		The udev stuff seems required for the board upload permissions
# 		cf. https://docs.platformio.org/en/latest/core/installation/udev-rules.html

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

RUN curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules \
 | tee /etc/udev/rules.d/99-platformio-udev.rules \
 && udevadm control --reload-rules \
 && udevadm trigger

RUN chmod +x ./container_cmd.sh

CMD ["/bin/bash", "./container_cmd.sh"]

# this can be exported like
#   docker build --target trophy-base -t trophy-base:latest .
#   docker save trophy-base:latest -o trophy-base.tar

# === Actual Builder ===
# can then load like
#   docker load -i trophy-base.tar
#   docker build --target builder -t trophy-base:latest .
# (technically, this runs the first stage again, but this is cached, so it doesn't)
#
#FROM trophy-base as trophy-builder
#
#WORKDIR /opt
#COPY --from=trophy-base /opt /opt
#
#COPY platformio_override.ini my_config.h* FX_DEADLINE_TROPHY.h* /opt/wled00/
#
#RUN platformio run
#
# RUN platformio run --target upload
#
#CMD /bin/bash
