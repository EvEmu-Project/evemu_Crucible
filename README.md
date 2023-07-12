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

## `docker-compose` Quickstart
 EVEmu can be run with Docker Compose:
```
git clone https://github.com/EvEmu-Project/evemu_Crucible.git
cd evemu_Crucible
docker-compose -p evemu up --build -d
```
**NOTE:** Add `--build` to the `docker-compose up` command to force a rebuild of the source. This is useful when making code changes.

By changing `build:` to `image:` in `docker-compose.yml`, you can use the prebuilt images available.

Configuration files are stored in the `evemu_config` volume. These can be modified and will persist across restarts.

To shutdown EVEmu:
```
docker-compose -p evemu down
```

## Building with Docker
 EVEmu can now be built with docker to ensure a consistent dependency base. This can be done by executing `docker build .` in the root directory.
 It is highly suggested to build EVEmu from the latest release available on the releases page.

 [Releases](https://github.com/EvEmu-Project/evemu_Crucible/releases)

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

