FROM wiiuenv/devkitppc:20220801

COPY --from=wiiuenv/libntfs:20220726 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmocha:20220726 /artifacts $DEVKITPRO

WORKDIR project
