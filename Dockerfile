# ==== Deadline Trophy Firmware Builder Image ====
#
# prepares the build environment with platformio and node
# the ultimate idea is that this is called as (mounting in your modifications in data/)
#   > podman run --rm -v ${PWD}/data:/mnt <name>
# see ./container_build.sh about the actual building process.
#
# note: based on the idea of the takigama/platformio base image
#       but they don't offer platformio 6.1.6 yet, which we require
#       (well, I didn't. but Korkenzieher did.)

FROM nikolaik/python-nodejs:python3.13-nodejs20

LABEL maintainer="qm210"

WORKDIR /home/pn/trophy
COPY --chown=pn:pn . /home/pn/trophy

RUN set -x && \
    npm install -g npm@latest

USER pn

ENV HOME=/home/pn
ENV PATH=$HOME/.local/bin:$HOME/.platformio/packages/tool-esptoolpy:${PATH}

RUN chmod +x ./container_build.sh && \
    # git config is only for less overlay spammy console output
    git config --global advice.detachedHead false && \
    pip install --user -U platformio && \
    # we fail horribly if we do not install esptool before the rest
    # (...took me about six very pleasurable hours to trace that)
    pio pkg install --tool platformio/tool-esptoolpy && \
    pio pkg install

CMD ["/bin/bash", "./container_build.sh"]
