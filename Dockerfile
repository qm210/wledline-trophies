# === Base Image ===
# prepares the build environment with python, platformio and node to build a .bin file.
#
# the ultimate idea is that this is called as (mounting in your modifications in data/)
#   > podman run --rm -v ${PWD}/data:/mnt <name>
#
# note: based on the idea of the takigama/platformio base image
#       but they don't offer platformio 6.1.6 yet, which we require
#       (well, I didn't. but Korkenzieher did.)

#FROM python:3
FROM nikolaik/python-nodejs:python3.13-nodejs20

# for debug output, but less spammypip install
RUN git config --global advice.detachedHead false
RUN set -x

RUN pip install -U platformio==6.1.18

# RUN apt-get update \
#  && apt-get install -y curl gnupg udev \
#  && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
#  && apt-get install -y nodejs \
#  && apt-get clean \
#  && rm -rf /var/lib/apt/lists/* \

RUN npm install -g npm@latest

WORKDIR /opt
COPY . /opt
RUN chmod +x ./container_build.sh

RUN platformio pkg install --project-dir .

CMD ["/bin/bash", "./container_build.sh"]
