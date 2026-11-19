@echo off
setlocal

rem Build the repository-patched EVEDBTool from the pinned upstream 0.0.6 source.
set "OUT=%~dp0evedbtool.exe"
set "ROOT=%~dp0.."
set "WORK=%TEMP%\EVEDBTool-0.0.6"

where git >nul 2>nul
if errorlevel 1 (
    echo ERROR: Git is required to build EVEDBTool.
    exit /b 1
)
where go >nul 2>nul
if errorlevel 1 (
    echo ERROR: Go 1.23 or newer is required to build EVEDBTool.
    exit /b 1
)

if exist "%WORK%" rmdir /s /q "%WORK%"
git clone --quiet --depth 1 --branch 0.0.6 https://github.com/EvEmu-Project/EVEDBTool.git "%WORK%"
if errorlevel 1 (
    echo ERROR: Could not clone the pinned EVEDBTool source.
    exit /b 1
)

copy /y "%ROOT%\tools\evedbtool\command_seed.go" "%WORK%\command_seed.go" >nul
if errorlevel 1 (
    echo ERROR: Could not apply the repository market-seed implementation.
    exit /b 1
)

pushd "%WORK%"
set CGO_ENABLED=0
set GOOS=windows
set GOARCH=amd64
go build -trimpath -ldflags "-s -w -X main.version=evemu-crucible" -o "%OUT%" .
set "BUILD_RESULT=%ERRORLEVEL%"
popd

if not "%BUILD_RESULT%"=="0" (
    echo ERROR: EVEDBTool build failed.
    exit /b %BUILD_RESULT%
)

if not exist "%OUT%" (
    echo ERROR: Build did not create evedbtool.exe.
    exit /b 1
)

echo Patched EVEDBTool built at:
echo %OUT%
echo.
echo Run it from the sql folder so it can find evedb.yaml and migrations.
exit /b 0
