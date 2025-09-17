/****************************************************************************
 * Copyright (C) 2021 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "WUDDumperState.h"
#include <WUD/content/WiiUDiscContentsHeader.h>
#include <common/common.h>
#include <fs/FSUtils.h>
#include <malloc.h>
#include <mocha/fsa.h>
#include <mocha/mocha.h>
#include <utils/StringTools.h>
#include <utils/WiiUScreen.h>
#include <utils/utils.h>

WUDDumperState::WUDDumperState(WUDDumperState::eDumpTargetFormat pTargetFormat, eDumpTarget pTargetDevice)
    : targetFormat(pTargetFormat), targetDevice(pTargetDevice) {
    this->sectorBufSize = READ_SECTOR_SIZE * READ_NUM_SECTORS;
    this->state         = STATE_OPEN_ODD1;
    gBlockHomeButton    = true;
    this->dumpStartDate = OSGetTime();
}

WUDDumperState::~WUDDumperState() {
    if (this->oddFd >= 0) {
        FSAEx_RawCloseEx(gFSAClientHandle, oddFd);
    }
    free(sectorBuf);
    free(emptySector);
    gBlockHomeButton = false;
}

ApplicationState::eSubState WUDDumperState::update(Input *input) {
    if (this->state == STATE_ERROR) {
        if (entrySelected(input)) {
            return ApplicationState::SUBSTATE_RETURN;
        }
        return ApplicationState::SUBSTATE_RUNNING;
    }
    if (this->state == STATE_OPEN_ODD1) {
        if (this->currentSector > 0) {
            auto ret = FSAEx_RawOpenEx(gFSAClientHandle, "/dev/odd01", &(this->oddFd));
            if (ret >= 0) {
                // continue!
                this->state = STATE_DUMP_DISC;
            } else {
                this->oddFd = -1;
                this->state = STATE_WAIT_USER_ERROR_CONFIRM;
            }
            return ApplicationState::SUBSTATE_RUNNING;
        }
        if (this->retryCount-- <= 0) {
            this->state = STATE_PLEASE_INSERT_DISC;
            return ApplicationState::SUBSTATE_RUNNING;
        }
        auto ret = FSAEx_RawOpenEx(gFSAClientHandle, "/dev/odd01", &(this->oddFd));
        if (ret >= 0) {
            if (this->sectorBuf == nullptr) {
                this->sectorBuf = (void *) memalign(0x100, this->sectorBufSize);
                if (this->sectorBuf == nullptr) {
                    DEBUG_FUNCTION_LINE_ERR("ERROR_MALLOC_FAILED");
                    this->setError(ERROR_MALLOC_FAILED);
                    return ApplicationState::SUBSTATE_RUNNING;
                }
            }
            DEBUG_FUNCTION_LINE("Opened /dev/odd01 %d", this->oddFd);
            this->state = STATE_READ_DISC_INFO;
        }
    } else if (this->state == STATE_PLEASE_INSERT_DISC) {
        if (entrySelected(input)) {
            return SUBSTATE_RETURN;
        }
    } else if (this->state == STATE_READ_DISC_INFO) {
        if (FSAEx_RawReadEx(gFSAClientHandle, this->sectorBuf, READ_SECTOR_SIZE, 1, 0, this->oddFd) >= 0) {
            this->discId[10] = '\0';
            memcpy(this->discId.data(), sectorBuf, 10);
            this->state = STATE_READ_DISC_INFO_DONE;
            return ApplicationState::SUBSTATE_RUNNING;
        }

        this->setError(ERROR_READ_FIRST_SECTOR);
        return ApplicationState::SUBSTATE_RUNNING;
    } else if (this->state == STATE_READ_DISC_INFO_DONE) {
        this->state = STATE_DUMP_DISC_KEY;
    } else if (this->state == STATE_DUMP_DISC_KEY) {
        // Read the WiiUDiscContentsHeader to determine if we need disckey and if it's the correct one.
        auto res = FSAEx_RawReadEx(gFSAClientHandle, this->sectorBuf, READ_SECTOR_SIZE, 1, 3, this->oddFd);
        WUDDiscKey discKey;
        bool hasDiscKey = false;
        if (res >= 0) {
            if (((uint32_t *) this->sectorBuf)[0] != WiiUDiscContentsHeader::MAGIC) {
                auto discKeyRes = Mocha_ODMGetDiscKey(&discKey);
                if (discKeyRes == MOCHA_RESULT_SUCCESS) {
                    hasDiscKey = true;
                }
            }
        }

        if (hasDiscKey) {
            if (!FSUtils::CreateSubfolder(string_format("%swudump/%s", getPathForDevice(targetDevice).c_str(), getPathNameForDisc().c_str()).c_str())) {
                setError(ERROR_WRITE_FAILED);
                return SUBSTATE_RUNNING;
            }
            if (!FSUtils::saveBufferToFile(string_format("%swudump/%s/game.key", getPathForDevice(targetDevice).c_str(), getPathNameForDisc().c_str()).c_str(), discKey.key, 16)) {
                setError(ERROR_WRITE_FAILED);
                return SUBSTATE_RUNNING;
            }
        }
        this->state = STATE_DUMP_DISC_START;
    } else if (this->state == STATE_DUMP_DISC_START) {
        if (!FSUtils::CreateSubfolder(string_format("%swudump/%s", getPathForDevice(targetDevice).c_str(), getPathNameForDisc().c_str()).c_str())) {
            setError(ERROR_WRITE_FAILED);
            return ApplicationState::SUBSTATE_RUNNING;
        }
        if (targetFormat == DUMP_AS_WUX) {
            this->fileHandle = std::make_unique<WUXFileWriter>(string_format("%swudump/%s/game.wux", getPathForDevice(targetDevice).c_str(), getPathNameForDisc().c_str()).c_str(), READ_SECTOR_SIZE * WRITE_BUFFER_NUM_SECTORS,
                                                               SECTOR_SIZE, targetDevice == TARGET_SD);
        } else {
            this->fileHandle = std::make_unique<WUDFileWriter>(string_format("%swudump/%s/game.wud", getPathForDevice(targetDevice).c_str(), getPathNameForDisc().c_str()).c_str(), READ_SECTOR_SIZE * WRITE_BUFFER_NUM_SECTORS,
                                                               SECTOR_SIZE, targetDevice == TARGET_SD);
        }
        if (!this->fileHandle->isOpen()) {
            DEBUG_FUNCTION_LINE_ERR("Failed to open file.");
            this->setError(ERROR_FILE_OPEN_FAILED);
            return ApplicationState::SUBSTATE_RUNNING;
        }

        this->startTime = OSGetTime();

        this->state            = STATE_DUMP_DISC;
        this->totalSectorCount = (WUD_FILE_SIZE / SECTOR_SIZE);
        this->currentSector    = 0;
        this->writtenSectors   = 0;
        this->retryCount       = 10;
    } else if (this->state == STATE_DUMP_DISC) {
        if (buttonPressed(input, Input::BUTTON_X)) {
            this->state = STATE_ABORT_CONFIRMATION;
            return ApplicationState::SUBSTATE_RUNNING;
        }

        size_t numSectors = this->currentSector + READ_NUM_SECTORS > this->totalSectorCount ? this->totalSectorCount - this->currentSector : READ_NUM_SECTORS;
        if ((this->readResult = FSAEx_RawReadEx(gFSAClientHandle, sectorBuf, READ_SECTOR_SIZE, numSectors, this->currentSector, this->oddFd)) >= 0) {
            auto curWrittenSectors = fileHandle->writeSector((const uint8_t *) this->sectorBuf, numSectors);
            if (curWrittenSectors < 0) {
                this->setError(ERROR_WRITE_FAILED);
                return ApplicationState::SUBSTATE_RUNNING;
            }
            currentSector += numSectors;
            this->writtenSectors += curWrittenSectors;

            this->retryCount = 10;
            if (this->currentSector >= this->totalSectorCount) {
                this->state = STATE_DUMP_DISC_DONE;
                if (this->fileHandle->isOpen()) {
                    if (!this->fileHandle->flush()) {
                        DEBUG_FUNCTION_LINE_ERR("Final flush failed");
                        this->setError(ERROR_WRITE_FAILED);
                        return ApplicationState::SUBSTATE_RUNNING;
                    }
                    if (!this->fileHandle->finalize()) {
                        DEBUG_FUNCTION_LINE_ERR("Finalize failed");
                        this->setError(ERROR_WRITE_FAILED);
                        return ApplicationState::SUBSTATE_RUNNING;
                    }
                    this->fileHandle->close();
                }
            }
        } else {
            this->state = STATE_WAIT_USER_ERROR_CONFIRM;
            if (this->oddFd >= 0) {
                FSAEx_RawCloseEx(gFSAClientHandle, this->oddFd);
                this->oddFd = -1;
            }
            return ApplicationState::SUBSTATE_RUNNING;
        }
    } else if (this->state == STATE_ABORT_CONFIRMATION) {

        if (buttonPressed(input, Input::BUTTON_B)) {
            this->state = STATE_DUMP_DISC;
            return ApplicationState::SUBSTATE_RUNNING;
        }
        proccessMenuNavigationX(input, 2);
        if (buttonPressed(input, Input::BUTTON_A)) {
            if (selectedOptionX == 0) {
                this->state = STATE_DUMP_DISC;
                return ApplicationState::SUBSTATE_RUNNING;
            } else {
                return ApplicationState::SUBSTATE_RETURN;
            }
        }
    } else if (this->state == STATE_WAIT_USER_ERROR_CONFIRM) {
        if (this->autoSkipOnError) {
            if (this->oddFd >= 0) {
                FSAEx_RawCloseEx(gFSAClientHandle, this->oddFd);
                this->oddFd = -1;
            }
        }
        if (this->autoSkipOnError || (input->data.buttons_d & Input::BUTTON_A)) {
            // this->log.fwrite("Skipped sector %d : 0x%ll016X-0x%ll016X, filled with 0's\n", this->currentSector, this->currentSector * READ_SECTOR_SIZE, (this->currentSector + 1) * READ_SECTOR_SIZE);
            this->state = STATE_OPEN_ODD1;
            this->skippedSectors.push_back(this->currentSector);
            // We can't use seek because we may have cached values.

            if (this->emptySector == nullptr) {
                this->emptySector = memalign(0x100, READ_SECTOR_SIZE);
                if (this->emptySector == nullptr) {
                    this->setError(ERROR_MALLOC_FAILED);
                    return ApplicationState::SUBSTATE_RUNNING;
                }
            }

            auto curWrittenSectors = fileHandle->writeSector((uint8_t *) emptySector, 1);
            if (curWrittenSectors < 0) {
                this->setError(ERROR_WRITE_FAILED);
                return ApplicationState::SUBSTATE_RUNNING;
            }

            this->currentSector += 1;
            this->writtenSectors += curWrittenSectors;
            this->readResult = 0;
        } else if (buttonPressed(input, Input::BUTTON_B)) {
            this->state      = STATE_OPEN_ODD1;
            this->readResult = 0;
        } else if (buttonPressed(input, Input::BUTTON_Y)) {
            this->autoSkipOnError = true;
        } else if (buttonPressed(input, Input::BUTTON_X)) {
            this->state = STATE_ABORT_CONFIRMATION;
            return ApplicationState::SUBSTATE_RUNNING;
        }
    } else if (this->state == STATE_DUMP_DISC_DONE) {
        WiiUScreen::drawLinef("Dumping done! Press A to continue");
        if (entrySelected(input)) {
            return SUBSTATE_RETURN;
        }

        return ApplicationState::SUBSTATE_RUNNING;
    }
    return ApplicationState::SUBSTATE_RUNNING;
}

void WUDDumperState::render() {
    WiiUScreen::clearScreen();
    ApplicationState::printHeader();
    if (this->state == STATE_ERROR) {
        WiiUScreen::drawLine();
        WiiUScreen::drawLinef("Error:       %s", ErrorMessage().c_str());
        WiiUScreen::drawLinef("Description: %s", ErrorDescription().c_str());
        WiiUScreen::drawLine();
        WiiUScreen::drawLine();
        WiiUScreen::drawLine("Press A to return.");
    } else if (this->state == STATE_OPEN_ODD1) {
        WiiUScreen::drawLine("Open /dev/odd01");
    } else if (this->state == STATE_PLEASE_INSERT_DISC) {
        WiiUScreen::drawLine("Please insert a Wii U disc and try again.\n\nPress A to return");
    } else if (this->state == STATE_DUMP_DISC_KEY) {
        WiiUScreen::drawLine("Read disc key");
    } else if (this->state == STATE_READ_DISC_INFO) {
        WiiUScreen::drawLine("Read disc information");
    } else if (this->state == STATE_READ_DISC_INFO_DONE) {
        WiiUScreen::drawLinef("Dumping: %s", getPathNameForDisc().c_str());
    } else if (this->state == STATE_DUMP_DISC_START || this->state == STATE_DUMP_DISC || this->state == STATE_WAIT_USER_ERROR_CONFIRM) {
        WiiUScreen::drawLinef("Dumping: %s", getPathNameForDisc().c_str());

        float percent = this->currentSector / (WUD_FILE_SIZE / READ_SECTOR_SIZE * 1.0f) * 100.0f;
        WiiUScreen::drawLinef("Progress: %0.2f MiB / %5.2f MiB (%2.1f %%)", this->currentSector * (READ_SECTOR_SIZE / 1024.0f / 1024.0f), WUD_FILE_SIZE / 1024.0f / 1024.0f, percent);
        if (targetFormat == DUMP_AS_WUX) {
            WiiUScreen::drawLinef("Written %0.2f MiB. Compression ratio 1:%0.2f", writtenSectors * (READ_SECTOR_SIZE / 1024.0f / 1024.0f),
                                  1.0f / (writtenSectors / (float) this->currentSector));
        }

        if (this->readResult < 0 || this->oddFd < 0) {
            WiiUScreen::drawLine();

            if (this->oddFd < 0) {
                WiiUScreen::drawLine("Failed to open disc, try again.");
            } else {
                WiiUScreen::drawLinef("Error: Failed to read sector - Error %d", this->readResult);
            }
            WiiUScreen::drawLine();
            WiiUScreen::drawLine("Press A to skip this sector (will be replaced by 0's)");
            WiiUScreen::drawLine("Press Y to activate auto-skip mode");
            WiiUScreen::drawLine("Press B to try again");
        } else {
            OSTime curTime       = OSGetTime();
            float remaining      = (WUD_FILE_SIZE - (READ_SECTOR_SIZE * this->currentSector)) / 1024.0f / 1024.0f;
            float curSpeed       = READ_SECTOR_SIZE * ((this->currentSector / 1000.0f) / OSTicksToMilliseconds(curTime - startTime));
            int32_t remainingSec = remaining / curSpeed;
            int32_t minutes      = (remainingSec / 60) % 60;
            int32_t seconds      = remainingSec % 60;
            int32_t hours        = remainingSec / 3600;

            WiiUScreen::drawLinef("Speed: %.2f MiB/s ETA: %02dh %02dm %02ds", curSpeed, remaining, hours, minutes, seconds);
        }

        WiiUScreen::drawLine();
        if (!this->skippedSectors.empty()) {
            WiiUScreen::drawLinef("Skipped dumping %d sectors", this->skippedSectors.size());
            WiiUScreen::drawLine();
        }
        WiiUScreen::drawLinef("Press X to abort");
    } else if (this->state == STATE_DUMP_DISC_DONE) {
        WiiUScreen::drawLinef("Dumping done! Press A to continue");
    } else if (this->state == STATE_ABORT_CONFIRMATION) {
        WiiUScreen::drawLinef("Do you really want to abort the disc dumping?");
        WiiUScreen::drawLinef("");
        if (selectedOptionX == 0) {
            WiiUScreen::drawLinef("> Continue dumping     Abort dumping");
        } else {
            WiiUScreen::drawLinef("  Continue dumping   > Abort dumping");
        }
    }

    ApplicationState::printFooter();
    WiiUScreen::flipBuffers();
}

void WUDDumperState::setError(WUDDumperState::eErrorState err) {
    this->state      = STATE_ERROR;
    this->errorState = err;
    //OSEnableHomeButtonMenu(true);
}

std::string WUDDumperState::ErrorMessage() const {
    switch (this->errorState) {
        case ERROR_READ_FIRST_SECTOR:
            return "ERROR_READ_FIRST_SECTOR";
        case ERROR_NONE:
            return "ERROR_NONE";
        case ERROR_FILE_OPEN_FAILED:
            return "ERROR_FILE_OPEN_FAILED";
        case ERROR_MALLOC_FAILED:
            return "ERROR_MALLOC_FAILED";
        case ERROR_WRITE_FAILED:
            return "ERROR_WRITE_FAILED";
        case ERROR_NO_DISC_FOUND:
            return "ERROR_NO_DISC_FOUND";
    }
    return "UNKNOWN_ERROR";
}

std::string WUDDumperState::ErrorDescription() const {
    switch (this->errorState) {
        case ERROR_READ_FIRST_SECTOR:
            return "Failed to read first sector.";
        case ERROR_NONE:
            return "ERROR_NONE";
        case ERROR_FILE_OPEN_FAILED:
            return "Failed to create file \nMake sure to have enough space on the storage device.";
        case ERROR_MALLOC_FAILED:
            return "Failed to allocate data.";
        case ERROR_WRITE_FAILED:
            return "Failed to write the file. \nMake sure to have enough space on the storage device.";
        case ERROR_NO_DISC_FOUND:
            return "Please insert a Wii U disc.";
    }
    return "UNKNOWN_ERROR";
}

std::string WUDDumperState::getPathForDevice(eDumpTarget target) const {
    if (target == TARGET_NTFS) {
        return "ntfs0:/";
    }
    return "fs:/vol/external01/";
}

std::string WUDDumperState::getPathNameForDisc() {
    if (this->discId[0] == '\0') {
        OSCalendarTime tm;
        OSTicksToCalendarTime(this->dumpStartDate, &tm);
        return string_format("DISC-%04d-%02d-%02d-%02d-%02d-%02d",
                             tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
                             tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    return std::string((char *) &discId[0]);
}
