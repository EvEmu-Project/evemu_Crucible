# [EVEmu](https://evemu.dev) - An EVE Online Emulator

<p align="center">
	<a href="https://github.com/EvEmu-Project/evemu_Crucible/pulse"><img src="https://img.shields.io/tokei/lines/github/EvEmu-Project/evemu_Crucible" /></a>
	<a href="https://www.codefactor.io/repository/github/evemu-project/evemu_crucible"><img src="https://img.shields.io/codefactor/grade/github/evemu-project/evemu_crucible" /></a>
	<a href="https://github.com/EvEmu-Project/evemu_Crucible/graphs/commit-activity"><img src="https://img.shields.io/github/commit-activity/w/EvEmu-Project/evemu_Crucible" /></a>
	<a href="https://github.com/EvEmu-Project/evemu_Crucible/graphs/contributors"><img src="https://img.shields.io/github/contributors/EvEmu-Project/evemu_Crucible" /></a>
	<a href="https://discord.gg/fTfAREYxbz"><img src="https://img.shields.io/discord/165291219205881856" /></a>
	<a href="https://github.com/EvEmu-Project/evemu_Crucible/issues"><img src="https://img.shields.io/github/issues-raw/EvEmu-Project/evemu_Crucible" /></a>
</p>

## Introduction
EVEmu is a work-in-progress server emulator for the space MMO EVE Online. This is an educational project. Please see the disclaimer below for details.

## ChangeLog
[ChangeLog](doc/ChangeLog.md)

## EVEmu Software Development Kit ##
The EVEmu project maintains a set of pre-configured tools and environments that help with setting up a new development station. [Check them out](https://github.com/EvEmu-Project/EvEmu_SDK)

Additional details on the SDK are [available on the wiki](https://wiki.evemu.dev/wiki/EVEmu-SDK).

## `docker compose` Quickstart
 EVEmu can be run with Docker Compose:
```
git clone https://github.com/EvEmu-Project/evemu_Crucible.git
cd evemu_Crucible
docker compose up -d
```
**NOTE:** Add `--build` to the `docker compose up` command to force a rebuild of the source. This is useful when making code changes.

~~By changing `build:` to `image:` in `docker-compose.yml`, you can use the prebuilt images available.~~

Configuration files are stored in `./config/`. These can be modified and will persist across restarts.

To shutdown EVEmu:
```
docker compose stop
```

## Building with Docker
 EVEmu can now be built with docker to ensure a consistent dependency base. This can be done by executing `docker compose build` in the root directory.
 It is highly suggested to build EVEmu from the latest release available on the releases page.

 [Releases](https://github.com/EvEmu-Project/evemu_Crucible/releases)

## Building with CMake and Ninja on Windows (using w64devkit)
@author:chuengeric

For developers who prefer a native Windows build without Docker, you can use [w64devkit](https://github.com/skeeto/w64devkit) along with the Ninja generator.

**Prerequisites:**
- Download and extract [w64devkit](https://github.com/skeeto/w64devkit/releases) to a folder, e.g., `C:\w64devkit`.
- Add `C:\w64devkit\mingw64\bin` to your system `PATH` (temporarily with `set PATH=C:\w64devkit\mingw64\bin;%PATH%` or permanently via Environment Variables).
- Verify the tools are available: `gcc --version`, `cmake --version`, `ninja --version`.


Download and extract w64devkit to a folder, e.g., C:\w64devkit. The recommended version is x86_64-16.2.0-release-mcf-seh-ucrt-rt_v14-rev1.7z (or later), which includes GCC 16.2.0, CMake 4.4.3, and Ninja 1.13.2. These versions have been tested and are known to work with this project.

Add C:\w64devkit\mingw64\bin to your system PATH (temporarily with set PATH=C:\w64devkit\mingw64\bin;%PATH% or permanently via Environment Variables).

Verify the tools are available by running the following commands and checking the output:

text
gcc --version      # should show 16.2.0 or similar
cmake --version    # should show 4.4.3 or later
ninja --version    # should show 1.13.2 or later
(updated 2012-09-07)


**Clean build (recommended):**
```cmd
cd C:\path\to\evemu_Crucible
rmdir /S build
mkdir build
cd build
Configure with CMake:
The following command uses -O0 (no optimization) to minimize memory consumption during compilation, which is helpful for development on resource-constrained machines.

**Note:** The following command uses `-O0` (no optimization) to minimize memory consumption during compilation, which is helpful for development on resource-constrained machines.

```bash
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -G "Ninja" -DCMAKE_C_COMPILER="C:/w64devkit/mingw64/bin/x86_64-w64-mingw32-gcc.exe" -DCMAKE_CXX_COMPILER="C:/w64devkit/mingw64/bin/x86_64-w64-mingw32-g++.exe" -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -DCMAKE_CXX_FLAGS="-O0 -g0 -fno-keep-inline-dllexport -fno-keep-static-consts -pipe -DHAVE_WINDOWS_H -DHAVE_WINSOCK2_H -IC:/w64devkit/mingw64/include" ..

Build the project:

bash
ninja
Or equivalently:

bash
cmake --build .
After a successful build, the executables (e.g., eve-server.exe) will be located in the build/ directory.

Note: If you have sufficient memory (>8 GB), you may change -O0 to -O2 for better runtime performance, but compilation will take longer and consume more RAM.

## Accounts
 Accounts will be created automatically when logging in with the client if the username is not already taken.

## Communication / Contact
 Check out the new [EVEmu Project website](https://evemu.dev), our [Discord](https://discord.gg/fTfAREYxbz) and [Forums](https://forums.evemu.dev)!

## Disclaimer
***EVEmu is an educational project.***
 This means, our primary interest is to learn and teach us
and our users more about C++ project development in a large
scale. Our software is not intended for running public servers,
and we do not support that. We are not responsible for what others
do with the source code downloaded from this project.

## Legal
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev/
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, see https://www.gnu.org/licenses/.
    ------------------------------------------------------------------------------------

