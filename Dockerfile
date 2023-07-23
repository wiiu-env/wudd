FROM ghcr.io/wiiu-env/devkitppc:20230621

COPY --from=ghcr.io/wiiu-env/libntfs:20220726 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmocha:20220919 /artifacts $DEVKITPRO

WORKDIR project
