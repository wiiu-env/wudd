#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_udp.h>

#include <thread>

#include <iosuhax.h>
#include <ntfs.h>
#include <coreinit/debug.h>
#include <coreinit/energysaver.h>

#include "utils/logger.h"
#include "utils/WiiUScreen.h"
#include "input/VPADInput.h"
#include "MainApplicationState.h"
#include "common/common.h"

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

    //DEBUG_FUNCTION_LINE("init fat");
    //fatInitDefault();

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

    main_loop();

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
    VPadInput input;

    if (gFSAfd < 0 || !sIosuhaxMount) {
        // state.setError(MainApplicationState::eErrorState::ERROR_IOSUHAX_FAILED);
        OSFatal("IOSUHAX Failed");
    }

    DEBUG_FUNCTION_LINE("Entering main loop");
    while (WHBProcIsRunning()) {
        input.update(1280, 720);
        state->update(&input);
        state->render();
    }
}

void initIOSUHax() {
    sIosuhaxMount = false;
    int res = IOSUHAX_Open(nullptr);
    if (res < 0) {
        DEBUG_FUNCTION_LINE("IOSUHAX_open failed");
    } else {
        sIosuhaxMount = true;
        gFSAfd = IOSUHAX_FSA_Open();
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