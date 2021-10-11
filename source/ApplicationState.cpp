#include "ApplicationState.h"
#include "utils/WiiUScreen.h"
#include "utils/ScreenUtils.h"
#include "common/common.h"
#include "utils/utils.h"
#include "utils/StringTools.h"
#include "utils/rijndael.h"
#include "fs/FSUtils.h"
#include <sysapp/launch.h>
#include <coreinit/ios.h>
#include <iosuhax.h>
#include <malloc.h>
#include <coreinit/memdefaultheap.h>
#include <cstdarg>
#include <ntfs.h>
#include <WUD/DiscReaderDiscDrive.h>
#include <whb/proc.h>
#include <coreinit/debug.h>
#include <WUD/content/partitions/WiiUGMPartition.h>
#include <WUD/header/WiiUDiscHeader.h>
#include <WUD/entities/TMD/Content.h>
#include <WUD/entities/TMD/TitleMetaData.h>
#include <WUD/NUSDataProviderWUD.h>
#include <WUD/NUSTitle.h>
#include <memory>

extern ntfs_md *ntfs_mounts;
extern int ntfs_mount_count;


unsigned int swap_uint32(unsigned int val) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

unsigned long long swap_uint64(unsigned long long val) {
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | (val >> 32);
}

/*
 * Hash function used to create a hash of each sector
 * The hashes are then compared to find duplicate sectors
 */
void calculateHash256(unsigned char *data, unsigned int length, unsigned char *hashOut) {
    // cheap and simple hash implementation
    // you can replace this part with your favorite hash method
    memset(hashOut, 0x00, 32);
    for (unsigned int i = 0; i < length; i++) {
        hashOut[i % 32] ^= data[i];
        hashOut[(i + 7) % 32] += data[i];
    }
}

ApplicationState::ApplicationState() : log("fs:/vol/external01/wudump.log", CFile::WriteOnly) {
    this->log.fwrite("Started wudump\n");
    this->state = STATE_WELCOME_SCREEN;
}

void ApplicationState::printHeader() {
    WiiUScreen::drawLine("Wudump");
    WiiUScreen::drawLine("==================");
    WiiUScreen::drawLine("");
}

void ApplicationState::render() {
    WiiUScreen::clearScreen();

    if (this->state == STATE_ERROR) {
        WiiUScreen::drawLine();
        WiiUScreen::drawLinef("Error:       %s", ErrorMessage().c_str());
        WiiUScreen::drawLinef("Description: %s", ErrorDescription().c_str());
        WiiUScreen::drawLine();
        WiiUScreen::drawLine("Press A to return to the Wii U Menu.");
    } else if (this->state == STATE_WELCOME_SCREEN) {
        WiiUScreen::drawLine("Welcome to Wudump");
        WiiUScreen::drawLine("Press A to dump the currently inserted Disc");
        WiiUScreen::drawLine("");
        if (this->selectedOption == 0) {
            WiiUScreen::drawLine("> Dump as WUX       Dump as WUD       Dump as .app       Exit");
        } else if (this->selectedOption == 1) {
            WiiUScreen::drawLine("  Dump as WUX     > Dump as WUD       Dump as .app       Exit");
        } else if (this->selectedOption == 2) {
            WiiUScreen::drawLine("  Dump as WUX       Dump as WUD     > Dump as .app       Exit");
        } else if (this->selectedOption == 3) {
            WiiUScreen::drawLine("  Dump as WUX       Dump as WUD       Dump as .app     > Exit");
        }
    } else if (this->state == STATE_CHOOSE_TARGET) {
        printHeader();
        WiiUScreen::drawLine("Please choose your target:");
        std::vector<std::string> options;
        int32_t targetCount = 0;
        if (this->dumpFormat == DUMP_AS_APP) {
            options.emplace_back("SD");
            targetCount++;
        }
        for (int i = 0; i < ntfs_mount_count; i++) {
            options.emplace_back(ntfs_mounts[i].name);
            targetCount++;
        }

        std::string curLine = "";
        if (this->selectedOption == 0) {
            curLine = "> Back\t";
        } else {
            curLine = "  Back\t";
        }

        if (targetCount == 0) {
            WiiUScreen::drawLine("Please insert a NTFS formatted USB drive and restart wudump\n");
        } else {
            for (int32_t i = 0; i < targetCount; i++) {
                if (this->selectedOption - 1 == i) {
                    curLine += "> " + options[i];
                } else {
                    curLine += "  " + options[i];
                }
                curLine += "\t";
            }
        }
        WiiUScreen::drawLine(curLine.c_str());
    } else if (this->state == STATE_OPEN_ODD1) {
        WiiUScreen::drawLine("Open /dev/odd01");
    } else if (this->state == STATE_READ_DISC_INFO) {
        WiiUScreen::drawLine("Read disc information");
    } else if (this->state == STATE_READ_DISC_INFO_DONE) {
        WiiUScreen::drawLinef("Dumping: %s", this->discId);
    } else if (this->state == STATE_DUMP_DISC_KEY) {
        WiiUScreen::drawLinef("Dumping game.key");
    } else if (this->state == STATE_DUMP_DISC_START || this->state == STATE_DUMP_DISC || this->state == STATE_WAIT_USER_ERROR_CONFIRM) {
        WiiUScreen::drawLinef("Dumping: %s", this->discId);

        float percent = this->currentSector / (WUD_FILE_SIZE / READ_SECTOR_SIZE * 1.0f) * 100.0f;
        WiiUScreen::drawLinef("Progress: %0.2f MiB / %5.2f MiB (%2.1f %%)", this->currentSector * (READ_SECTOR_SIZE / 1024.0f / 1024.0f), WUD_FILE_SIZE / 1024.0f / 1024.0f, percent);
        if (doWUX) {
            WiiUScreen::drawLinef("Written %0.2f MiB. Compression ratio 1:%0.2f", this->hashMap.size() * (READ_SECTOR_SIZE / 1024.0f / 1024.0f),
                                  1.0f / (this->hashMap.size() / (float) this->currentSector));
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
            WiiUScreen::drawLine("Press B to try again");
        } else {
            OSTime curTime = OSGetTime();
            float remaining = (WUD_FILE_SIZE - (READ_SECTOR_SIZE * this->currentSector)) / 1024.0f / 1024.0f;
            float curSpeed = READ_SECTOR_SIZE * ((this->readSectors / 1000.0f) / OSTicksToMilliseconds(curTime - startTime));
            int32_t remainingSec = remaining / curSpeed;
            int32_t minutes = (remainingSec / 60) % 60;
            int32_t seconds = remainingSec % 60;
            int32_t hours = remainingSec / 3600;

            WiiUScreen::drawLinef("Speed: %.2f MiB/s ETA: %02dh %02dm %02ds", curSpeed, remaining, hours, minutes, seconds);
        }

        WiiUScreen::drawLine();
        if (!this->skippedSectors.empty()) {
            WiiUScreen::drawLinef("Skipped dumping %d sectors", this->skippedSectors.size());
        }
    } else if (this->state == STATE_DUMP_DISC_DONE) {
        if (!flushWriteCache()) {
            setError(ERROR_WRITE_FAILED);
        }
        WiiUScreen::drawLinef("Dumping done! Press A to continue");
    } else if (this->state == STATE_DUMP_APP_FILES_DONE) {
        WiiUScreen::drawLinef("Dumping done! Press A to continue");
    } else if (this->state == STATE_PLEASE_INSERT_DISC) {
        WiiUScreen::drawLinef("Please insert a Disc. Press A to continue");
    }
    printFooter();
    WiiUScreen::flipBuffers();
}

void ApplicationState::update(Input *input) {
    if (this->state == STATE_ERROR) {
        OSEnableHomeButtonMenu(true);
        if (entrySelected(input)) {
            SYSLaunchMenu();
        }
    } else if (this->state == STATE_WELCOME_SCREEN) {
        proccessMenuNavigation(input, 4);
        if (entrySelected(input)) {
            if (this->selectedOption == 0) {
                this->retryCount = 10;
                this->state = STATE_CHOOSE_TARGET;
                this->dumpFormat = DUMP_AS_WUX;
            } else if (this->selectedOption == 1) {
                this->retryCount = 10;
                this->state = STATE_CHOOSE_TARGET;
                this->dumpFormat = DUMP_AS_WUD;
            } else if (this->selectedOption == 2) {
                this->retryCount = 10;
                this->state = STATE_CHOOSE_TARGET;
                this->dumpFormat = DUMP_AS_APP;
            } else {
                SYSLaunchMenu();
            }
            this->selectedOption = 0;
            return;
        }
    } else if (this->state == STATE_CHOOSE_TARGET) {
        WiiUScreen::drawLine("Please choose your target");
        std::vector<std::string> options;
        uint32_t targetCount = 0;

        if (this->dumpFormat == DUMP_AS_APP) {
            options.emplace_back("fs:/vol/external01/");
            targetCount++;
        }
        if (ntfs_mount_count > 0) {

            for (int i = 0; i < ntfs_mount_count; i++) {
                options.emplace_back(std::string(ntfs_mounts[i].name) + ":/");
                targetCount++;
            }
        }

        proccessMenuNavigation(input, targetCount + 1);
        if (entrySelected(input)) {
            if (this->selectedOption == 0) {
                this->state = STATE_WELCOME_SCREEN;
            } else if (targetCount > 0) {
                target = options[selectedOption - 1];
                this->state = STATE_OPEN_ODD1;
            }
            this->selectedOption = 0;
        }
    } else if (this->state == STATE_OPEN_ODD1) {
        if (this->readSectors > 0) {
            auto ret = IOSUHAX_FSA_RawOpen(gFSAfd, "/dev/odd01", &(this->oddFd));
            if (ret >= 0) {
                // continue!
                this->state = STATE_DUMP_DISC;
            } else {
                this->oddFd = -1;
                this->state = STATE_WAIT_USER_ERROR_CONFIRM;
            }
            return;
        }
        DEBUG_FUNCTION_LINE("STATE_OPEN_ODD1");
        if (this->retryCount-- <= 0) {
            this->state = STATE_PLEASE_INSERT_DISC;
            return;
        }
        auto ret = IOSUHAX_FSA_RawOpen(gFSAfd, "/dev/odd01", &(this->oddFd));
        if (ret >= 0) {
            if (this->sectorBuf == nullptr) {
                this->sectorBuf = (void *) memalign(0x100, this->sectorBufSize);
                if (this->sectorBuf == nullptr) {
                    this->setError(ERROR_MALLOC_FAILED);
                    return;
                }
            }
            DEBUG_FUNCTION_LINE("Opened /dev/odd01 %d", this->oddFd);
            this->state = STATE_READ_DISC_INFO;
        }
    } else if (this->state == STATE_READ_DISC_INFO) {
        DEBUG_FUNCTION_LINE("STATE_READ_DISC_INFO");
        if (IOSUHAX_FSA_RawRead(gFSAfd, this->sectorBuf, READ_SECTOR_SIZE, 1, 0, this->oddFd) >= 0) {
            this->discId[10] = '\0';
            memcpy(this->discId, sectorBuf, 10);
            if (this->discId[0] == 0) {
                setError(ERROR_NO_DISC_ID);
                return;
            }
            this->state = STATE_READ_DISC_INFO_DONE;
            return;
        }

        this->setError(ERROR_READ_FIRST_SECTOR);
        return;
    } else if (this->state == STATE_READ_DISC_INFO_DONE) {
        DEBUG_FUNCTION_LINE("STATE_READ_DISC_INFO_DONE");
        this->state = STATE_DUMP_DISC_KEY;
    } else if (this->state == STATE_DUMP_DISC_KEY) {
        DEBUG_FUNCTION_LINE("STATE_DUMP_DISC_KEY");

        auto res = IOSUHAX_FSA_RawRead(gFSAfd, this->sectorBuf, READ_SECTOR_SIZE, 1, 3, this->oddFd);
        uint8_t discKey[16];
        bool hasDiscKey = false;
        if (res >= 0) {
            if (((uint32_t *) this->sectorBuf)[0] != 0xCCA6E67B) {
                auto discKeyRes = IOSUHAX_ODM_GetDiscKey(discKey);
                if (discKeyRes >= 0) {
                    hasDiscKey = true;
                }
            }
        }

        if (hasDiscKey) {
            if (!FSUtils::CreateSubfolder(StringTools::fmt("%swudump/%s", target.c_str(), discId))) {
                setError(ERROR_WRITE_FAILED);
                return;
            }
            if (!FSUtils::saveBufferToFile(StringTools::fmt("%swudump/%s/game.key", target.c_str(), discId), discKey, 16)) {
                setError(ERROR_WRITE_FAILED);
                return;
            }
        }
        if (this->dumpFormat == DUMP_AS_WUX || this->dumpFormat == DUMP_AS_WUD) {
            if (this->dumpFormat == DUMP_AS_WUX) {
                this->doWUX = true;
            }
            this->state = STATE_DUMP_DISC_START;
        } else {
            this->state = STATE_DUMP_APP_FILES;
        }
    } else if (this->state == STATE_DUMP_APP_FILES) {
        ApplicationState::dumpAppFiles();
        if (this->state != STATE_ERROR) {
            this->state = STATE_DUMP_APP_FILES_DONE;
        }
    } else if (this->state == STATE_DUMP_APP_FILES_DONE) {
        if (entrySelected(input)) {
            this->state = STATE_WELCOME_SCREEN;
            if (this->oddFd >= 0) {
                IOSUHAX_FSA_RawClose(gFSAfd, this->oddFd);
                this->oddFd = -1;
            }
            this->currentSector = 0;
            this->readSectors = 0;
        }
    } else if (this->state == STATE_DUMP_DISC_START) {
        ApplicationState::clearWriteCache();
        DEBUG_FUNCTION_LINE("STATE_DUMP_DISC_START");
        if (!FSUtils::CreateSubfolder(StringTools::fmt("%swudump/%s", target.c_str(), discId))) {
            setError(ERROR_WRITE_FAILED);
            return;
        }
        this->fileHandle = new CFile(StringTools::fmt("%swudump/%s/game.%s", target.c_str(), discId, doWUX ? "wux" : "wud"), CFile::WriteOnly);

        this->totalSectorCount = WUD_FILE_SIZE / SECTOR_SIZE;

        if (!this->fileHandle->isOpen()) {
            DEBUG_FUNCTION_LINE("Failed to open file");
            this->setError(ERROR_FILE_OPEN_FAILED);
            return;
        }

        if (doWUX) {
            wuxHeader_t wuxHeader = {0};
            wuxHeader.magic0 = WUX_MAGIC_0;
            wuxHeader.magic1 = WUX_MAGIC_1;
            wuxHeader.sectorSize = swap_uint32(SECTOR_SIZE);
            wuxHeader.uncompressedSize = swap_uint64(WUD_FILE_SIZE);
            wuxHeader.flags = 0;

            DEBUG_FUNCTION_LINE("Write header");
            this->fileHandle->write((uint8_t *) &wuxHeader, sizeof(wuxHeader_t));
            this->sectorTableStart = this->fileHandle->tell();

            this->sectorIndexTable = (void *) malloc(totalSectorCount * 4);
            if (sectorIndexTable == nullptr) {
                this->setError(ERROR_MALLOC_FAILED);
                return;
            }
            memset(this->sectorIndexTable, 0, totalSectorCount * 4);

            DEBUG_FUNCTION_LINE("Write empty sectorIndexTable");
            this->fileHandle->write((uint8_t *) this->sectorIndexTable, totalSectorCount * 4);

            DEBUG_FUNCTION_LINE("Get sector table end");
            this->sectorTableEnd = this->fileHandle->tell();
            uint64_t tableEnd = this->sectorTableEnd;

            this->sectorTableEnd += SECTOR_SIZE - 1;
            this->sectorTableEnd -= (this->sectorTableEnd % SECTOR_SIZE);

            uint64_t padding = this->sectorTableEnd - tableEnd;
            auto *paddingData = (uint8_t *) malloc(padding);
            memset(paddingData, 0, padding);
            this->fileHandle->write(reinterpret_cast<const uint8_t *>(paddingData), padding);
            free(paddingData);
            this->hashMap.clear();
        }

        this->writeBufferSize = READ_SECTOR_SIZE * WRITE_BUFFER_NUM_SECTORS;
        this->writeBuffer = (void *) memalign(0x1000, this->writeBufferSize);
        if (this->writeBuffer == nullptr) {
            this->setError(ERROR_MALLOC_FAILED);
            return;
        }
        this->writeBufferPos = 0;

        this->startTime = OSGetTime();

        this->state = STATE_DUMP_DISC;
        this->currentSector = 0;
        this->retryCount = 10;
        this->selectedOption = 0;
        this->readSectors = 0;
    } else if (this->state == STATE_DUMP_DISC) {
        //DEBUG_FUNCTION_LINE("STATE_DUMP_DISC");
        int32_t numSectors = this->currentSector + READ_NUM_SECTORS > this->totalSectorCount ? this->totalSectorCount - this->currentSector : READ_NUM_SECTORS;
        if ((this->readResult = IOSUHAX_FSA_RawRead(gFSAfd, sectorBuf, READ_SECTOR_SIZE, numSectors, this->currentSector, this->oddFd)) >= 0) {
            if (!writeDataToFile(this->sectorBuf, numSectors)) {
                this->setError(ERROR_WRITE_FAILED);
                return;
            }
            //DEBUG_FUNCTION_LINE("Read done %lld %lld", this->currentSector, this->totalSectorCount);
            this->retryCount = 10;
            if (this->currentSector >= this->totalSectorCount) {
                this->state = STATE_DUMP_DISC_DONE;

                if (this->fileHandle->isOpen()) {
                    if (!this->flushWriteCache()) {
                        this->setError(ERROR_WRITE_FAILED);
                        return;
                    }
                    if (doWUX) {
                        this->writeSectorIndexTable();
                    }
                    this->fileHandle->close();
                }
            }
        } else {
            this->state = STATE_WAIT_USER_ERROR_CONFIRM;
            if (this->oddFd >= 0) {
                IOSUHAX_FSA_RawClose(gFSAfd, this->oddFd);
                this->oddFd = -1;
            }
            return;
        }
    } else if (this->state == STATE_WAIT_USER_ERROR_CONFIRM) {
        if (this->autoSkip) {
            if (this->oddFd >= 0) {
                IOSUHAX_FSA_RawClose(gFSAfd, this->oddFd);
                this->oddFd = -1;
            }
        }
        if (this->autoSkip || (input->data.buttons_d & Input::BUTTON_A)) {
            this->log.fwrite("Skipped sector %d : 0x%ll016X-0x%ll016X, filled with 0's\n", this->currentSector, this->currentSector * READ_SECTOR_SIZE, (this->currentSector + 1) * READ_SECTOR_SIZE);
            this->state = STATE_OPEN_ODD1;
            this->skippedSectors.push_back(this->currentSector);
            // We can't use seek because we may have cached values.

            if (this->emptySector == nullptr) {
                this->emptySector = memalign(0x100, READ_SECTOR_SIZE);
                if (this->emptySector == nullptr) {
                    this->setError(ERROR_MALLOC_FAILED);
                    return;
                }
            }

            if (!this->writeCached(reinterpret_cast<uint32_t>(emptySector), READ_SECTOR_SIZE)) {
                this->setError(ERROR_WRITE_FAILED);
                return;
            }

            this->currentSector += 1;
            this->readResult = 0;
        } else if (input->data.buttons_d & Input::BUTTON_B) {
            this->state = STATE_OPEN_ODD1;
            this->readResult = 0;
        } else if (input->data.buttons_d & Input::BUTTON_Y) {
            this->autoSkip = true;
        }
    } else if (this->state == STATE_DUMP_DISC_DONE) {
        if (entrySelected(input)) {
            if (this->oddFd >= 0) {
                IOSUHAX_FSA_RawClose(gFSAfd, this->oddFd);
                this->oddFd = -1;
            }
            this->state = STATE_WELCOME_SCREEN;
            this->selectedOption = 0;
            this->currentSector = 0;
            this->readSectors = 0;
            this->writtenSector = 0;
            return;
        }
    } else if (this->state == STATE_PLEASE_INSERT_DISC){
        if (entrySelected(input)) {
            this->state = STATE_WELCOME_SCREEN;
        }
    }
}

std::string ApplicationState::ErrorMessage() {
    if (this->error == ERROR_NONE) {
        return "NONE";
    } else if (this->error == ERROR_IOSUHAX_FAILED) {
        return "ERROR_IOSUHAX_FAILED";
    } else if (this->error == ERROR_MALLOC_FAILED) {
        return "ERROR_MALLOC_FAILED";
    } else if (this->error == ERROR_FILE_OPEN_FAILED) {
        return "ERROR_FILE_OPEN_FAILED";
    } else if (this->error == ERROR_NO_DISC_ID) {
        return "ERROR_NO_DISC_ID";
    }
    DEBUG_FUNCTION_LINE("Error: %d", this->error);
    return "UNKNOWN_ERROR";
}

std::string ApplicationState::ErrorDescription() {
    if (this->error == ERROR_NONE) {
        return "-";
    } else if (this->error == ERROR_IOSUHAX_FAILED) {
        return "Failed to init IOSUHAX.";
    } else if (this->error == ERROR_MALLOC_FAILED) {
        return "Failed to allocate data.";
    } else if (this->error == ERROR_FILE_OPEN_FAILED) {
        return "Failed to create file";
    } else if (this->error == ERROR_NO_DISC_ID) {
        return "Failed to get the disc id";
    }
    DEBUG_FUNCTION_LINE("Error: %d", this->error);
    return "UNKNOWN_ERROR";
}

void ApplicationState::setError(eErrorState err) {
    this->state = STATE_ERROR;
    this->error = err;
    //OSEnableHomeButtonMenu(true);
}

void ApplicationState::printFooter() {
    ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_TV, 0, 27, "By Maschell");
    ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_DRC, 0, 17, "By Maschell");
}

void ApplicationState::proccessMenuNavigation(Input *input, int maxOptionValue) {
    if (input->data.buttons_d & Input::BUTTON_LEFT) {
        this->selectedOption--;
    } else if (input->data.buttons_d & Input::BUTTON_RIGHT) {
        this->selectedOption++;
    }
    if (this->selectedOption < 0) {
        this->selectedOption = maxOptionValue;
    } else if (this->selectedOption >= maxOptionValue) {
        this->selectedOption = 0;
    }
}

bool ApplicationState::entrySelected(Input *input) {
    return input->data.buttons_d & Input::BUTTON_A;
}

ApplicationState::~ApplicationState() {
    this->log.close();
    if (this->fileHandle->isOpen()) {
        if (!this->flushWriteCache()) {

        }
        if (doWUX) {
            this->writeSectorIndexTable();
        }
        this->fileHandle->close();
    }
    if (this->emptySector != nullptr) {
        free(this->emptySector);
        this->emptySector = nullptr;
    }
    if (this->writeBuffer != nullptr) {
        free(this->writeBuffer);
        this->writeBuffer = nullptr;
    }
    if (this->sectorIndexTable != nullptr) {
        free(this->sectorIndexTable);
        this->sectorIndexTable = nullptr;
    }
    if (this->sectorBuf != nullptr) {
        free(this->sectorBuf);
        this->sectorBuf = nullptr;
    }
    if (this->oddFd >= 0) {
        IOSUHAX_FSA_RawClose(gFSAfd, this->oddFd);
        this->oddFd = -1;
    }
}

bool ApplicationState::writeDataToFile(void *buffer, int numberOfSectors) {
    if (!doWUX) {
        if (!writeCached(reinterpret_cast<uint32_t>(buffer), numberOfSectors * READ_SECTOR_SIZE)) {
            return false;
        }
        this->currentSector += numberOfSectors;
        this->readSectors += numberOfSectors;
    } else {
        char hashOut[32];
        for (int i = 0; i < numberOfSectors; i++) {
            uint32_t addr = ((uint32_t) buffer) + (i * READ_SECTOR_SIZE);
            calculateHash256(reinterpret_cast<unsigned char *>(addr), READ_SECTOR_SIZE, reinterpret_cast<unsigned char *>(hashOut));
            char tmp[34];
            auto *test = (uint32_t *) hashOut;
            snprintf(tmp, 33, "%08X%08X%08X%08X", test[0], test[1], test[2], test[3]);
            std::string hash(tmp);

            uint32_t *indexTable = (uint32_t *) this->sectorIndexTable;

            auto it = hashMap.find(hash);
            if (it != hashMap.end()) {
                indexTable[this->currentSector] = swap_uint32(this->hashMap[hash]);
            } else {
                indexTable[this->currentSector] = swap_uint32(this->writtenSector);
                hashMap[hash] = this->writtenSector;
                if (this->fileHandle->isOpen()) {
                    if (!writeCached(addr, READ_SECTOR_SIZE)) {
                        return false;
                    }
                }
                this->writtenSector++;
            }
            this->currentSector++;
            this->readSectors++;
        }
    }
    return true;
}

bool ApplicationState::writeCached(uint32_t addr, uint32_t writeSize) {
    // DEBUG_FUNCTION_LINE("Lest write %d bytes", writeSize);

    if (writeSize == this->writeBufferSize) {
        if (!this->flushWriteCache()) {
            return false;
        }
        int32_t res = this->fileHandle->write(reinterpret_cast<const uint8_t *>(addr), writeSize);
        return res >= 0;
    }

    uint32_t toWrite = writeSize;
    if (toWrite == 0) {
        return true;
    }

    uint32_t written = 0;

    do {
        uint32_t curWrite = toWrite;

        if (this->writeBufferPos + curWrite > this->writeBufferSize) {
            curWrite = this->writeBufferSize - this->writeBufferPos;
        }
        // DEBUG_FUNCTION_LINE("Copy from %08X into %08X, size %08X, %d",(addr + written),((uint32_t) this->writeBuffer) + this->writeBufferPos, curWrite, this->writeBufferPos/READ_SECTOR_SIZE);
        OSBlockMove((void *) (((uint32_t) this->writeBuffer) + this->writeBufferPos), (void *) (addr + written), curWrite, 1);
        this->writeBufferPos += curWrite;

        if (this->writeBufferPos == this->writeBufferSize) {
            if (!flushWriteCache()) {
                return false;
            }
        }

        toWrite -= curWrite;
        written += curWrite;
    } while (toWrite > 0);
    return true;
}

bool ApplicationState::flushWriteCache() {
    if (this->writeBufferPos > 0) {
        int32_t res = this->fileHandle->write(static_cast<const uint8_t *>(this->writeBuffer), this->writeBufferPos);
        if (res < 0) {
            return false;
        }
        this->writeBufferPos = 0;
    }
    return true;
}

void ApplicationState::clearWriteCache() {
    this->writeBufferPos = 0;
}

void ApplicationState::writeSectorIndexTable() {
    if (this->fileHandle->isOpen() && doWUX) {
        this->fileHandle->seek(this->sectorTableStart, SEEK_SET);
        this->fileHandle->write((uint8_t *) this->sectorIndexTable, totalSectorCount * 4);
    }
}

void ApplicationState::printDumpState(const char *fmt, ...) {
    WiiUScreen::clearScreen();
    ApplicationState::printHeader();
    char *buf = (char *) MEMAllocFromDefaultHeapEx(PRINTF_BUFFER_LENGTH, 4);
    va_list va;

    if (!buf) {
        return;
    }

    va_start(va, fmt);
    vsnprintf(buf, PRINTF_BUFFER_LENGTH, fmt, va);

    WiiUScreen::drawLine(buf);

    MEMFreeToDefaultHeap(buf);
    va_end(va);
    ApplicationState::printFooter();
    WiiUScreen::flipBuffers();
}

void ApplicationState::dumpAppFiles() {
    uint8_t opt[0x400];
    IOSUHAX_read_otp(opt, 0x400);
    std::array<uint8_t, 0x10> cKey{};
    memcpy(cKey.data(), opt + 0xE0, 0x10);

    DEBUG_FUNCTION_LINE("Reading Partitions");

    printDumpState("Reading Partitions...");

    auto discReader = std::make_shared<DiscReaderDiscDrive>();
    if (!discReader->IsReady()) {
        DEBUG_FUNCTION_LINE("!IsReady");
        this->setError(ERROR_OPEN_ODD1);
        return;
    }
    DEBUG_FUNCTION_LINE("Read DiscHeader");
    auto discHeader = WiiUDiscHeader::make_unique(discReader);
    if (!discHeader.has_value()) {
        DEBUG_FUNCTION_LINE("Failed to read discheader");
        return;
    }
    bool forceExit = false;
    for (auto &partition: discHeader.value()->wiiUContentsInformation->partitions->partitions) {
        auto gmPartition = std::dynamic_pointer_cast<WiiUGMPartition>(partition);
        if (gmPartition != nullptr) {
            auto nusTitleOpt = NUSTitle::loadTitleFromGMPartition(gmPartition, discReader, cKey);
            if (!nusTitleOpt.has_value()) {
                DEBUG_FUNCTION_LINE("nusTitle was null");
                continue;
            }
            auto nusTitle = nusTitleOpt.value();
            auto dataProvider = nusTitle->dataProcessor->getDataProvider();

            uint64_t partitionSize = 0;
            uint64_t partitionSizeWritten = 0;
            for (auto &content: nusTitle->tmd->contentList) {
                partitionSize += ROUNDUP(content->encryptedFileSize, 16);
            }

            auto partitionDumpInfo = StringTools::strfmt("Partition: %s\n\tProgress: %.2f MiB / %.2f MiB\n", partition->getVolumeId().c_str(), partitionSizeWritten / 1024.0f / 1024.0f,
                                                         partitionSize / 1024.0f / 1024.0f);
            printDumpState("%s", partitionDumpInfo.c_str());

            char buffer[512];
            snprintf(buffer, 500, "%swudump/%s/%s", target.c_str(), this->discId, gmPartition->getVolumeId().c_str());
            FSUtils::CreateSubfolder(buffer);

            std::vector<uint8_t> wBuffer;
            if (dataProvider->getRawTMD(wBuffer)) {
                std::string fileName = std::string(buffer).append("/").append(WUD_TMD_FILENAME);
                printDumpState("%s\nSaving %s", partitionDumpInfo.c_str(), WUD_TMD_FILENAME);
                FSUtils::saveBufferToFile(fileName.c_str(), wBuffer.data(), wBuffer.size());
                wBuffer.clear();
            }

            if (dataProvider->getRawTicket(wBuffer)) {
                std::string fileName = std::string(buffer).append("/").append(WUD_TICKET_FILENAME);
                printDumpState("%s\nSaving %s", partitionDumpInfo.c_str(), WUD_TICKET_FILENAME);
                FSUtils::saveBufferToFile(fileName.c_str(), wBuffer.data(), wBuffer.size());
                wBuffer.clear();
            }

            if (dataProvider->getRawCert(wBuffer)) {
                std::string fileName = std::string(buffer).append("/").append(WUD_TICKET_FILENAME);
                printDumpState("%s\nSaving %s", partitionDumpInfo.c_str(), WUD_CERT_FILENAME);
                FSUtils::saveBufferToFile(fileName.c_str(), wBuffer.data(), wBuffer.size());
                wBuffer.clear();
            }

            auto contentCount = nusTitle->tmd->contentList.size();
            auto contentI = 1;

            for (auto &content: nusTitle->tmd->contentList) {
                char bufApp[32];
                snprintf(bufApp, 31, "%08X.app", content->ID);
                std::string appFileName = std::string(buffer) + "/" + bufApp;

                partitionDumpInfo = StringTools::strfmt("Partition: %s\n\tProgress: %.2f MiB / %.2f MiB\n", partition->getVolumeId().c_str(), partitionSizeWritten / 1024.0f / 1024.0f,
                                                        partitionSize / 1024.0f / 1024.0f);
                auto contentDumpInfo = StringTools::strfmt("Saving %s (Content %02d/%02d)\n", bufApp, contentI, contentCount);

                printDumpState("%s\n%s", partitionDumpInfo.c_str(), contentDumpInfo.c_str());

                uint32_t bufferSize = READ_NUM_SECTORS * READ_SECTOR_SIZE * 2;

                auto *readBuffer = (uint8_t *) malloc(bufferSize);
                if (readBuffer == nullptr) {
                    DEBUG_FUNCTION_LINE("Failed to alloc buffer");
                    continue;
                }

                CFile file(appFileName, CFile::WriteOnly);
                if (!file.isOpen()) {
                    free(readBuffer);
                    continue;
                }
                uint32_t readSoFar = 0;
                uint64_t curOffset = 0;
                uint32_t size = ROUNDUP(content->encryptedFileSize, 16);
                OSTime startTimeApp = OSGetTime();
                do {
                    if (!WHBProcIsRunning()) {
                        forceExit = true;
                        break;
                    }
                    startTimeApp = OSGetTime();
                    WiiUScreen::clearScreen();

                    uint32_t toRead = size - readSoFar;
                    if (toRead > bufferSize) {
                        toRead = bufferSize;
                    }
                    dataProvider->readRawContent(content, readBuffer, curOffset, toRead);
                    if (file.write((const uint8_t *) readBuffer, toRead) != (int32_t) toRead) {
                        DEBUG_FUNCTION_LINE("Failed to write");
                        break;
                    }

                    OSTime curTime = OSGetTime();
                    auto curSpeed = (float) toRead / (float) OSTicksToMilliseconds(curTime - startTimeApp);

                    readSoFar += toRead;
                    curOffset += toRead;

                    partitionSizeWritten += toRead;

                    partitionDumpInfo = StringTools::strfmt("Partition: %s\n\tProgress: %.2f MiB / %.2f MiB\n", partition->getVolumeId().c_str(), partitionSizeWritten / 1024.0f / 1024.0f,
                                                            partitionSize / 1024.0f / 1024.0f);
                    printDumpState("%s\n%s\tProgress: %.2f MiB / %.2f MiB (%0.2f%%)\n\tSpeed: %0.2f MiB/s", partitionDumpInfo.c_str(), contentDumpInfo.c_str(), readSoFar / 1024.0f / 1024.0f,
                                   size / 1024.0f / 1024.0f, ((readSoFar * 1.0f) / size) * 100.0f, curSpeed / 1024.0f);
                } while (readSoFar < size);

                file.close();

                if (forceExit) {
                    break;
                }

                std::vector<uint8_t> h3Data;
                if (dataProvider->getContentH3Hash(content, h3Data)) {
                    char bufh3[32];
                    snprintf(bufh3, 31, "%08X.h3", content->ID);
                    std::string h3FileName = std::string(buffer) + "/" + bufh3;
                    printDumpState("%s\n%s", partitionDumpInfo.c_str(), contentDumpInfo.c_str());
                    FSUtils::saveBufferToFile(h3FileName.c_str(), h3Data.data(), h3Data.size());
                }
                contentI++;
            }
            if (forceExit) {
                exit(0);
                break;
            }
        }
    }
}
