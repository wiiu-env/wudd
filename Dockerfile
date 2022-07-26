FROM wiiuenv/devkitppc:20220724

COPY --from=wiiuenv/libiosuhax:20220523 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libntfs:20201210 /artifacts $DEVKITPRO

WORKDIR project