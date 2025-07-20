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

# RUN apt-get update \
#  && apt-get install -y curl gnupg udev \
#  && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
#  && apt-get install -y nodejs \
#  && apt-get clean \
#  && rm -rf /var/lib/apt/lists/* \

RUN npm install -g npm@latest

WORKDIR /opt
COPY . /opt

# WIP: could be good style to run as non-root.
#      -- but is more work, for some future person, who loves stuff like that.
RUN chown -R pn:pn /opt
USER pn

RUN echo && echo && echo
ENV HOME="/home/pn"
RUN echo "HOME="$HOME
ENV PATH="$HOME/.local/bin:$HOME/.platformio/packages/tool-esptoolpy:${PATH}"
RUN echo "PATH="$PATH

RUN mkdir -p $HOME/.platformio

RUN pip install --user -U platformio

# this is not only debug output, it also creates .../.platformio
# RUN platformio system info

RUN platformio pkg install --tool platformio/tool-esptoolpy
RUN platformio pkg install

RUN chmod +x ./container_build.sh

CMD ["/bin/bash", "./container_build.sh"]
