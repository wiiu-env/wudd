# WUDD - Wii U Disc Dumper

Inspired by [wudump](https://github.com/FIX94/wudump) from FIX94.

Features:
- Dump a Wii U Disc in WUD (uncompressed) or [WUX](https://gbatemp.net/threads/wii-u-image-wud-compression-tool.397901/) (loseless compression) format (including the game.key)
- Dump the GM Partitions (Game, Updates, DLCs) of an WiiU Disc as *.app,*.h3, .tmd, .tik, .cert files
- Supports dumping to SD (FAT32) and USB (NTFS only). When dumping to SD the files get slitted in 2 GiB parts. 

Files will be dumped to `/wudump/[DISC-ID]/`. The DiscID of a game can be found on the disc (e.g. WUP-P-ARDP for the EUR version of Super Mario 3D World).

## How to merge splitted files

When you dump a .wux or .wud to the SD card it gets splitted into 2 GiB parts (FAT32 limitation). To merge them you can use the `copy` cmd tool.

Example:
`copy /b "game.wux.part1" + "game.wux.part2" "C:\wudump\game.wux"`

## Dependencies

- [wut](https://github.com/devkitPro/wut)
- [libiosuhax](https://github.com/wiiu-env/libiosuhax)
- [libntfs](https://github.com/wiiu-env/libntfs)

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

`docker run --rm -v ${PWD}:/src wiiuenv/clang-format:13.0.0-2 -r ./source -i`