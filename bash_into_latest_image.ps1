# Get the most recently created image ID
$ImageId = podman images --format "{{.ID}}" --sort created | Select-Object -First 1

# Now run a shell in that image (or use your preferred command)
if ($imageId) {
    podman run -it $imageId powershell
} else {
    Write-Host "No image found"
}