# WUDD - Wii U Disc Dumper

Inspired by [wudump](https://github.com/FIX94/wudump) from FIX94.

Features:
- Dump a Wii U Disc in WUD (uncompressed) or [WUX](https://gbatemp.net/threads/wii-u-image-wud-compression-tool.397901/) (loseless compression) format (including the game.key)
- Dump the GM Partitions (Game, Updates, DLCs) of an Wii U Disc as *.app,*.h3, .tmd, .tik, .cert files
- Supports dumping to SD (FAT32) and USB (NTFS only). When dumping to SD the files get slitted in 2 GiB parts. 

Files will be dumped to `/wudump/[DISC-ID]/`. The disc id of a game can be found on the disc (e.g. WUP-P-ARDP for the EUR version of Super Mario 3D World). If WUDD fails to determine the disc id, "DISC" with a timestamp will be used instead.

If you want to create a partial dump (skipped sectors represented by 00 bytes) for discs with unreadable sectors, you can avoid the need to manually choose the skip sectors option on each error, by pressing Y at the error message to activate auto skip mode

## How to merge splitted files

When you dump a .wux or .wud to the SD card it gets splitted into 2 GiB parts (FAT32 limitation). To merge them you can use the `copy` cmd tool.

Example:
`copy /b game.wux.part01 + game.wux.part02 game.wux`

## Dependencies
Requires an [Environment](https://github.com/wiiu-env/EnvironmentLoader) (e.g. Tiramisu or Aroma) with [MochaPayload](https://github.com/wiiu-env/MochaPayload) (Nightly-MochaPayload-20220725-155554 or newer)

- [wut](https://github.com/devkitPro/wut)
- [libmocha](https://github.com/wiiu-env/libmocha)
- [libntfs](https://github.com/wiiu-env/libntfs)

## Buildflags

### Logging
Building via `make` only logs errors (via OSReport). To enable logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule) set `DEBUG` to `1` or `VERBOSE`.

`make` Logs errors only (via OSReport).  
`make DEBUG=1` Enables information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  
`make DEBUG=VERBOSE` Enables verbose information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) is not present, it'll fallback to UDP (Port 4405) and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t wudd-builder

# make 
docker run -it --rm -v ${PWD}:/project wudd-builder make

# make clean
docker run -it --rm -v ${PWD}:/project wudd-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./source -i`
