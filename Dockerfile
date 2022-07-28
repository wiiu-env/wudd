FROM wiiuenv/devkitppc:20220728

COPY --from=wiiuenv/libntfs:20220726 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmocha:20220726 /artifacts $DEVKITPRO

WORKDIR project