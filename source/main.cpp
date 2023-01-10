#include "MainApplicationState.h"
#include "input/VPADInput.h"
#include "utils/WiiUScreen.h"
#include "utils/logger.h"
#include <coreinit/debug.h>
#include <coreinit/energysaver.h>
#include <coreinit/title.h>
#include <input/CombinedInput.h>
#include <input/WPADInput.h>
#include <mocha/disc_interface.h>
#include <mocha/mocha.h>
#include <ntfs.h>
#include <padscore/kpad.h>
#include <sndcore2/core.h>
#include <thread>
#include <whb/log.h>
#include <whb/proc.h>

void initMochaLib();

void deInitMochaLib();

void main_loop();

bool slibMochaMount = false;

extern "C" void ACPInitialize();

static uint32_t
procHomeButtonDeniedCustom(void *context) {
    if (!gBlockHomeButton && gRunFromHBL) {
        WHBProcStopRunning();
    } else {
        gBlockHomeButtonCooldown = 5;
    }
    return 0;
}

int main(int argc, char **argv) {
    initLogging();
    AXInit();
    WHBProcInit();
    WiiUScreen::Init();

    initMochaLib();
    ACPInitialize();

    uint64_t titleID = OSGetTitleID();
    if (titleID == 0x0005000013374842 ||
        titleID == 0x000500101004A000 ||
        titleID == 0x000500101004A100 ||
        titleID == 0x000500101004A200) {
        gRunFromHBL = true;

        ProcUIClearCallbacks();
        ProcUIRegisterCallback(PROCUI_CALLBACK_HOME_BUTTON_DENIED,
                               &procHomeButtonDeniedCustom, nullptr, 100);
    } else {
        gRunFromHBL = false;
    }

    uint32_t isAPDEnabled;
    IMIsAPDEnabled(&isAPDEnabled);

    if (isAPDEnabled) {
        DEBUG_FUNCTION_LINE_VERBOSE("Disable auto shutdown");
        IMDisableAPD();
    }

    ntfs_mount_count = ntfsMountAll((ntfs_md **) &ntfs_mounts, NTFS_DEFAULT | NTFS_RECOVER);

    for (int i = 0; i < ntfs_mount_count; i++) {
        DEBUG_FUNCTION_LINE("%s:", ntfs_mounts[i].name);
    }

    WPADInput::init();

    gFSAClientHandle = FSAAddClient(nullptr);
    if (!gFSAClientHandle) {
        OSFatal("FSAAddClient failed");
    }

    if (Mocha_UnlockFSClientEx(gFSAClientHandle) != MOCHA_RESULT_SUCCESS) {
        OSFatal("Failed to unlock FSAClientHandle");
    }

    main_loop();

    FSADelClient(gFSAClientHandle);

    WPADInput::close();

    if (ntfs_mounts != nullptr) {
        int i;
        for (i = 0; i < ntfs_mount_count; i++) {
            ntfsUnmount(ntfs_mounts[i].name, true);
        }
        free(ntfs_mounts);
    }

    if (isAPDEnabled) {
        DEBUG_FUNCTION_LINE_VERBOSE("Enable auto shutdown");
        IMEnableAPD();
    }

    deInitMochaLib();

    WiiUScreen::DeInit();
    WHBProcShutdown();
    AXQuit();

    return 0;
}

void main_loop() {
    DEBUG_FUNCTION_LINE_VERBOSE("Creating state");
    std::unique_ptr<MainApplicationState> state = std::make_unique<MainApplicationState>();
    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[4] = {
            WPAD_CHAN_0,
            WPAD_CHAN_1,
            WPAD_CHAN_2,
            WPAD_CHAN_3};

    DEBUG_FUNCTION_LINE_VERBOSE("Entering main loop");
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

void initMochaLib() {
    slibMochaMount       = false;
    MochaUtilsStatus res = Mocha_InitLibrary();
    if (res != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Mocha_InitLibrary failed: %s", Mocha_GetStatusStr(res));
        OSFatal("Failed to init libmocha. Please update MochaPayload.");
    } else {
        slibMochaMount = true;
    }
}

void deInitMochaLib() {
    if (slibMochaMount) {
        Mocha_DeInitLibrary();
    }

    Mocha_sdio_disc_interface.shutdown();
    Mocha_usb_disc_interface.shutdown();
}