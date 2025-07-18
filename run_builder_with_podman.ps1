param(
    [string]$Port
)

# as an example of how it can look
# if the previous "podman build -t trophy-builder ." went through successfully
podman run -it --rm -v $PWD/data:/mnt trophy-builder

# if given the $Port, then try upload (assumes that the build went through)
if ($PSBoundParameters.ContainsKey('Port')) {
    esptool --port $Port write-flash -z 0x10000 .\data\firmware.bin
}
