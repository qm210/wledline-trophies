param(
    [string]$Name = "trophy-builder",
    [string]$Port
)

# as an example of how all of this can look

$imageExists = podman images --format "{{.Repository}}" | Where-Object { $_ -eq $Name }
if (-not $imageExists) {
    podman build -t $Name .
}

podman run -it --rm -v $PWD/data:/mnt $Name

# if given the $Port, then try upload (assumes that the build went through)
if ($PSBoundParameters.ContainsKey('Port')) {
    esptool --port $Port write-flash -z 0x10000 .\data\firmware.bin
}
