$ImageId = podman images --filter "dangling=true" --format "{{.ID}}" | Select-Object -First 1

if (-not $ImageId) {
    Write-Host "There is no dangling image."
    exit 1
}

podman run -it --rm $ImageId /bin/bash
