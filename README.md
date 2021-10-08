# Wudump

RPX version of [wudump](https://github.com/FIX94/wudump).

Supports dumping as [WUX](https://gbatemp.net/threads/wii-u-image-wud-compression-tool.397901/) and WUD, but only to NTFS formatted USB drives and without dumping the `game.key`.

## Dependencies

- [wut](https://github.com/decaf-emu/wut)
- [libiosuhax](https://github.com/wiiu-env/libiosuhax)
- [libfat](https://github.com/wiiu-env/libfat)
- [libntfs](https://github.com/wiiu-env/libntfs)

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t wudump-builder

# make 
docker run -it --rm -v ${PWD}:/project wudump-builder make

# make clean
docker run -it --rm -v ${PWD}:/project wudump-builder make clean
```