# Sync and build on Raspberry Pi
param(
    [string]$PiHost = "pi@192.168.1.100",  # Change to your Pi's IP
    [string]$PiPath = "~/keeperfx"
)

Write-Host "Syncing files to Raspberry Pi..." -ForegroundColor Cyan

# Sync using rsync (install with: choco install rsync or use WSL)
wsl rsync -avz --delete --exclude 'obj/' --exclude 'bin/' --exclude '.git/' . ${PiHost}:${PiPath}/

Write-Host "Building on Raspberry Pi..." -ForegroundColor Green
ssh $PiHost "cd $PiPath && make -f linux.mk all -j`$(nproc)"

Write-Host "Done! Downloading binary..." -ForegroundColor Yellow
scp ${PiHost}:${PiPath}/bin/keeperfx ./bin/

Write-Host "Build complete!" -ForegroundColor Green
