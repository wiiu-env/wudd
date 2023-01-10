FROM wiiuenv/devkitppc:20221228

COPY --from=wiiuenv/libntfs:20220726 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmocha:20220919 /artifacts $DEVKITPRO

WORKDIR project
