@echo off
setlocal

rem Download the pinned EVEDBTool Windows x86_64 release.
set "URL=https://github.com/EvEmu-Project/EVEDBTool/releases/download/0.0.6/evedbtool.exe"
set "OUT=%~dp0evedbtool.exe"

echo Downloading EVEDBTool 0.0.6 for Windows...
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ProgressPreference='SilentlyContinue'; Invoke-WebRequest -UseBasicParsing -Uri '%URL%' -OutFile '%OUT%'"

if errorlevel 1 (
    echo ERROR: EVEDBTool download failed.
    exit /b 1
)

if not exist "%OUT%" (
    echo ERROR: Download did not create evedbtool.exe.
    exit /b 1
)

for %%F in ("%OUT%") do if %%~zF LSS 1000000 (
    echo ERROR: Downloaded file is unexpectedly small.
    del "%OUT%"
    exit /b 1
)

echo EVEDBTool downloaded to:
echo %OUT%
echo.
echo Run it from the sql folder so it can find evedb.yaml and migrations.
exit /b 0
