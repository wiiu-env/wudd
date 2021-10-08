FROM wiiuenv/devkitppc:20210920

COPY --from=wiiuenv/libiosuhax:20211008 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libntfs:20201210 /artifacts $DEVKITPRO

WORKDIR project