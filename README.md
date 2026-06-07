# MarsRobot

This a Robot Demo for running F Prime on a Pi-compatible Linux platform.
The goal is to illustrate how to set up an F Prime project in a tangible way for people getting started using the framework.

[F´ (F Prime)](https://fprime.jpl.nasa.gov) is a component-driven framework that enables rapid development and deployment of spaceflight and other embedded software applications.

## Pololu Romi

The Romi was selected as the plaftorm for this project, largely because the parts are available and easily purchased.
Specific consideration was given to minimizing the amount design work and striving for relatively low assembly difficulty, in order to focus on the software learning.

![Pololu ROMI](img/ROMI_bot_500px.jpg)

- [Pololu Romi Chassis Kit](https://www.pololu.com/product/3504) - Ours were white and yellow, but any color will do
- [Pololu Romi Encoder Pair Kit](https://www.pololu.com/product/3542) - Requires soldering but a no-solder motor/encoder pair is available
- [Pololu Romi 32U4 Control Board](https://www.pololu.com/product/3544) - Provides and interesting and programmable Atmega34u4 to communicate with
- Raspberry Pi with WiFi access - Wifi setup is beyond the scope of this repo, we used a 3B+ and 4B
- 6 AA Batteries - Your local grocery or home improvement store

The code running on the Romi's Atmega32u4 is located in the ``arduino_code`` folder.
For our purposes, we treat is as a black box sub-system that we can control from a Raspberry Pi over I2C.
Uploading the firmare to the Romi is a straightforward Arduino upload process, even though it requires some libraries from Pololu for the Romi32u4 platform.

## Building for Raspberry Pi 4 (aarch64)

The deployment targets a Raspberry Pi 4 running 64-bit Debian Bookworm (aarch64).
The recommended build path is a Docker container running Ubuntu 24.04, which
provides the `aarch64-linux-gnu` cross-compiler without any host-platform setup.

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) and Docker Compose
- A Pi 4 sysroot synced from the target board (see *Sysroot* below)

### Sysroot

The build links against headers and libraries from the target Pi rather than
the cross-compiler's bundled copies.  Sync the sysroot once from a running Pi:

```bash
rsync -avzhe ssh --progress pi@<PI_IP>:/lib      sysroot/
rsync -avzhe ssh --progress pi@<PI_IP>:/usr/include sysroot/usr/
rsync -avzhe ssh --progress pi@<PI_IP>:/usr/lib    sysroot/usr/
```

A pre-synced copy from a Bookworm Pi 4 is kept at `sysroot/` (gitignored).

### Build

```bash
# Build the Docker image and cross-compile the deployment (one command):
docker compose -f docker/docker-compose.yml run --rm build

# Or explicitly as:
docker compose -f docker/docker-compose.yml run --rm build bash -c fprime-util generate && fprime-util build

# Alternatively this sequence allows the last command to be repeated for smaller
# incremental changes:
docker compose -f docker/docker-compose.yml run --rm build bash -c fprime-util purge --force
docker compose -f docker/docker-compose.yml run --rm build bash -c fprime-util generate
docker compose -f docker/docker-compose.yml run --rm build bash -c fprime-util build

# The binary is produced at:
#   build-fprime-automatic-Linux/MarsRobot/MarsRobot
```

### Interactive shell inside the build container

```bash
docker compose -f docker/docker-compose.yml run --rm build bash

# Inside the container:
fprime-util generate aarch64-linux -DCMAKE_SYSROOT=/project/sysroot
fprime-util build -j$(nproc)
```

### Deploy to Pi

```bash
scp build-fprime-automatic-Linux/MarsRobot/MarsRobot pi@<PI_IP>:~/
```
