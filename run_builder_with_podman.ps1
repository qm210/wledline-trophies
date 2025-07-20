param(
    [string]$Name = "trophy-builder",
    [string]$Port
)

# as an example of how all of this can look

$imageExists = podman images --format "{{.Repository}}" | Where-Object { $_ -eq $Name }
if (-not $imageExists) {
    podman build -t $Name .
}

# colorful xterm is 256 colorful! :)
podman run --rm -it -e TERM=xterm-256color -v $PWD/data:/mnt $Name

# for debugging, hack into the container as
# podman run -it --rm -v $PWD/data:/mnt $Name /bin/bash

# if given the $Port, then try upload (assumes that the build went through)
# NOTE: this is experimental, rather use the WLED-internal OTA Software Updater!
if ($PSBoundParameters.ContainsKey('Port')) {
    esptool --port $Port write-flash -z 0x10000 .\data\firmware.bin
}
