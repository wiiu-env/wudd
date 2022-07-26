#include <whb/log.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

#include <thread>

#include <coreinit/debug.h>
#include <coreinit/energysaver.h>
#include <coreinit/title.h>
#include <input/CombinedInput.h>
#include <input/WPADInput.h>
#include <iosuhax.h>
#include <ntfs.h>
#include <padscore/kpad.h>

#include "MainApplicationState.h"
#include "input/VPADInput.h"
#include "utils/WiiUScreen.h"
#include "utils/logger.h"


void initIOSUHax();

void deInitIOSUHax();

void main_loop();

bool sIosuhaxMount = false;

int main(int argc, char **argv) {
    WHBLogUdpInit();
    DEBUG_FUNCTION_LINE("Hello from wudump!");
    WHBProcInit();
    WiiUScreen::Init();

    initIOSUHax();

    uint64_t titleID = OSGetTitleID();
    if (titleID == 0x0005000013374842 ||
        titleID == 0x000500101004A000 ||
        titleID == 0x000500101004A100 ||
        titleID == 0x000500101004A200) {
        gRunFromHBL = true;
    } else {
        gRunFromHBL = false;
    }

    uint32_t isAPDEnabled;
    IMIsAPDEnabled(&isAPDEnabled);

    if (isAPDEnabled) {
        DEBUG_FUNCTION_LINE("Disable auto shutdown");
        IMDisableAPD();
    }

    ntfs_mount_count = ntfsMountAll((ntfs_md **) &ntfs_mounts, NTFS_DEFAULT | NTFS_RECOVER);

    for (int i = 0; i < ntfs_mount_count; i++) {
        DEBUG_FUNCTION_LINE("%s:", ntfs_mounts[i].name);
    }

    WPADInput::init();

    main_loop();

    WPADInput::close();

    if (ntfs_mounts != nullptr) {
        int i = 0;
        for (i = 0; i < ntfs_mount_count; i++) {
            ntfsUnmount(ntfs_mounts[i].name, true);
        }
        free(ntfs_mounts);
    }

    if (isAPDEnabled) {
        DEBUG_FUNCTION_LINE("Enable auto shutdown");
        IMEnableAPD();
    }

    deInitIOSUHax();

    WiiUScreen::DeInit();
    WHBProcShutdown();

    return 0;
}

void main_loop() {
    DEBUG_FUNCTION_LINE("Creating state");
    std::unique_ptr<MainApplicationState> state = std::make_unique<MainApplicationState>();
    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[4] = {
            WPAD_CHAN_0,
            WPAD_CHAN_1,
            WPAD_CHAN_2,
            WPAD_CHAN_3};

    if (gFSAfd < 0 || !sIosuhaxMount) {
        // state.setError(MainApplicationState::eErrorState::ERROR_IOSUHAX_FAILED);
        OSFatal("IOSUHAX Failed");
    }

    DEBUG_FUNCTION_LINE("Entering main loop");
    while (WHBProcIsRunning()) {
        baseInput.reset();
        if (vpadInput.update(1280, 720)) {
            baseInput.combine(vpadInput);
        }
        for (auto &wpadInput : wpadInputs) {
            if (wpadInput.update(1280, 720)) {
                baseInput.combine(wpadInput);
            }
        }
        baseInput.process();
        state->update(&baseInput);
        state->render();
    }
}

void initIOSUHax() {
    sIosuhaxMount = false;
    int res       = IOSUHAX_Open(nullptr);
    if (res < 0) {
        DEBUG_FUNCTION_LINE("IOSUHAX_open failed");
    } else {
        sIosuhaxMount = true;
        gFSAfd        = IOSUHAX_FSA_Open();
        if (gFSAfd < 0) {
            DEBUG_FUNCTION_LINE("IOSUHAX_FSA_Open failed");
        } else {
            DEBUG_FUNCTION_LINE("IOSUHAX done");
        }
    }
}

void deInitIOSUHax() {
    if (sIosuhaxMount) {
        if (gFSAfd >= 0) {
            IOSUHAX_FSA_Close(gFSAfd);
        }
        IOSUHAX_Close();
    }
}