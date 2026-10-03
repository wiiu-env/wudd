# WUDD - Wii U Disc Dumper

Inspired by [wudump](https://github.com/FIX94/wudump) by FIX94.

## Features

* Dump a Wii U disc in WUD (uncompressed) or [WUX](https://gbatemp.net/threads/wii-u-image-wud-compression-tool.397901/) (lossless compression) format, including the game key.
* Dump the GM partitions (Game, Updates, and DLC) of a Wii U disc as `.app`, `.h3`, `.tmd`, `.tik`, and `.cert` files.
* Supports dumping to SD (FAT32) and USB (NTFS only). When dumping to an SD card, files are split into 2 GiB parts due to the FAT32 file-size limitation.
* Save disc hashes to the SD card. **Hash disc** hashes the disc without saving any disc data.

Files are dumped to `/wudump/[DISC-ID]/`.

The disc ID of a game can be found on the disc (e.g. `WUP-P-ARDP` for the EUR version of Super Mario 3D World). If WUDD cannot determine the disc ID, `DISC` followed by a timestamp is used instead.

If you want to create a partial dump (with skipped sectors represented by `00` bytes) for discs with unreadable sectors, you can avoid manually selecting the skip-sectors option for each error by pressing **Y** when an error occurs. This activates auto-skip mode.

## How to Merge Split Files

When you dump a `.wux` or `.wud` file to an SD card, it is split into 2 GiB parts due to the FAT32 file-size limitation.

To merge the parts, you can use the Windows `copy` command:

```cmd
copy /b game.wux.part01 + game.wux.part02 game.wux
```

## Dependencies

WUDD requires an [Environment](https://github.com/wiiu-env/EnvironmentLoader) (e.g. Tiramisu or Aroma) with [MochaPayload](https://github.com/wiiu-env/MochaPayload) (Nightly-MochaPayload-20220725-155554 or newer).

Build dependencies:

* [wut](https://github.com/devkitPro/wut)
* [libmocha](https://github.com/wiiu-env/libmocha)
* [libntfs](https://github.com/wiiu-env/libntfs)

## Build Flags

### Logging

Building with `make` only logs errors via `OSReport`. To enable logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule), set `DEBUG` to `1` or `VERBOSE`.

* `make` — Logs errors only via `OSReport`.
* `make DEBUG=1` — Enables information and error logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule).
* `make DEBUG=VERBOSE` — Enables verbose information and error logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule).

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) is not present, WUDD falls back to UDP logging on port `4405` and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.

## Building Using the Dockerfile

You can use the provided Dockerfile to build WUDD inside a Docker container. This means you don't need to install the build dependencies on your host system.

```sh
# Build the Docker image (only needed once)
docker build . -t wudd-builder

# Build
docker run -it --rm -v ${PWD}:/project wudd-builder make

# Clean
docker run -it --rm -v ${PWD}:/project wudd-builder make clean
```

## Formatting the Code Using Docker

You can format the source code using the provided ClangFormat Docker image:

```sh
docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./source -i
```
