FROM ghcr.io/wiiu-env/devkitppc:20260504

COPY --from=ghcr.io/wiiu-env/libntfs:20220726 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmocha:20260331 /artifacts $DEVKITPRO

WORKDIR project
