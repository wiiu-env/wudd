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
#include <common/common.h>
#include <iosuhax.h>
#include <memory>
#include <WUD/DiscReaderDiscDrive.h>
#include <WUD/header/WiiUDiscHeader.h>
#include <fs/FSUtils.h>
#include <WUD/content/partitions/WiiUGMPartition.h>
#include <WUD/NUSTitle.h>
#include "GMPartitionsDumperState.h"
#include <utils/StringTools.h>

#define READ_BUFFER_SIZE (SECTOR_SIZE * 128)

GMPartitionsDumperState::GMPartitionsDumperState(eDumpTarget pTargetDevice) : targetDevice(pTargetDevice) {
    this->sectorBufSize = SECTOR_SIZE;
    this->state = STATE_OPEN_ODD1;
}

GMPartitionsDumperState::~GMPartitionsDumperState() {
    free(this->sectorBuf);
    this->sectorBuf = nullptr;
    free(this->readBuffer);
    this->readBuffer = nullptr;
}

void GMPartitionsDumperState::render() {
    WiiUScreen::clearScreen();
    ApplicationState::printHeader();

    if (this->state == STATE_ERROR) {
        WiiUScreen::drawLine();
        WiiUScreen::drawLinef("Error:       %s", ErrorMessage().c_str());
        WiiUScreen::drawLinef("Description: %s", ErrorDescription().c_str());
        WiiUScreen::drawLine();
        WiiUScreen::drawLine("Press A to return.");
    } else if (this->state == STATE_OPEN_ODD1) {
        WiiUScreen::drawLine("Open /dev/odd01");
    } else if (this->state == STATE_PLEASE_INSERT_DISC) {
        WiiUScreen::drawLine("Please insert a Wii U disc and try again.\n\nPress A to return");
    } else if (this->state == STATE_READ_DISC_INFO) {
        WiiUScreen::drawLine("Read disc information");
    } else if (this->state == STATE_READ_DISC_INFO_DONE) {
        WiiUScreen::drawLine("Read disc information done");
    } else if (this->state == STATE_READ_COMMON_KEY) {
        WiiUScreen::drawLine("Read common key");
    } else if (this->state == STATE_CREATE_DISC_READER) {
        WiiUScreen::drawLine("Create disc reader");
    } else if (this->state == STATE_PARSE_DISC_HEADER) {
        WiiUScreen::drawLine("Parse disc header");
    } else if (this->state == STATE_PROCESS_GM_PARTITIONS) {
        WiiUScreen::drawLine("Process partitions");
    } else if (this->state == STATE_CHOOSE_PARTITION_TO_DUMP) {
        WiiUScreen::drawLine("Choose the partition to dump:");
        WiiUScreen::drawLine();
        if (gmPartitionPairs.empty()) {
            WiiUScreen::drawLine("This disc has no dumpable GM partitions ");
        } else {
            uint32_t index = 0;
            for (auto &partitionPair: gmPartitionPairs) {
                uint32_t size = 0;
                for (auto &content: partitionPair.second->tmd->contentList) {
                    size += ROUNDUP(content->encryptedFileSize, 16);
                }
                WiiUScreen::drawLinef("%s %s (~%0.2f MiB)", index == (uint32_t) selectedOptionY ? ">" : " ", partitionPair.first->getVolumeId().c_str(), (float) size / 1024.0f / 1024.0f);
                index++;
            }
            WiiUScreen::drawLine();
            WiiUScreen::drawLinef("%s Back", index == (uint32_t) selectedOptionY ? ">" : " ");
        }
    } else if (this->state == STATE_CREATE_DATA_PROVIDER) {
        WiiUScreen::drawLine("Create data provider from partition");
    } else if (this->state == STATE_DUMP_PARTITION_TMD) {
        WiiUScreen::drawLine("Dump title.tmd");
    } else if (this->state == STATE_DUMP_PARTITION_TICKET) {
        WiiUScreen::drawLine("Dump title.tik");
    } else if (this->state == STATE_DUMP_PARTITION_CERT) {
        WiiUScreen::drawLine("Dump title.cert");
    } else if (this->state == STATE_DUMP_PARTITION_CONTENTS) {
        if (curPartition != nullptr) {
            WiiUScreen::drawLinef("Dumping Partition %s", curPartition->getVolumeId().c_str());
        } else {
            WiiUScreen::drawLine("Dumping Partition");
        }

        uint64_t partitionSize = 0;
        uint64_t partitionOffset = curOffsetInContent;
        if (curNUSTitle != nullptr) {
            for (auto &content: curNUSTitle->tmd->contentList) {
                partitionSize += ROUNDUP(content->encryptedFileSize, 16);
                if (content->index < curContentIndex) {
                    partitionOffset += ROUNDUP(content->encryptedFileSize, 16);
                }
            }
            WiiUScreen::drawLinef("Total Progress: %.2f MiB / %.2f MiB (%0.2f%%)", partitionOffset / 1024.0f / 1024.0f,
                                  partitionSize / 1024.0f / 1024.0f, ((partitionOffset * 1.0f) / partitionSize) * 100.0f);
            WiiUScreen::drawLine();
        } else {
            WiiUScreen::drawLine();
            WiiUScreen::drawLine();
        }

        if (curNUSTitle != nullptr) {
            WiiUScreen::drawLinef("Dumping Content %d / %d", curContentIndex, curNUSTitle->tmd->contentList.size());
        } else {
            WiiUScreen::drawLine("Dumping Content ?? / ??");
        }

        auto offset = curOffsetInContent;
        auto size = curContent != nullptr ? ROUNDUP(curContent->encryptedFileSize, 16) : 0;

        if (size > 0) {
            WiiUScreen::drawLinef("Progress: %.2f MiB / %.2f MiB (%0.2f%%)", offset / 1024.0f / 1024.0f,
                                  size / 1024.0f / 1024.0f, ((offset * 1.0f) / size) * 100.0f);
        }

    } else if (this->state == STATE_DUMP_DONE) {
        WiiUScreen::drawLine("Dumping done. Press A to return.");
    }

    ApplicationState::printFooter();
    WiiUScreen::flipBuffers();
}

ApplicationState::eSubState GMPartitionsDumperState::update(Input *input) {
    if (this->state == STATE_RETURN) {
        return ApplicationState::SUBSTATE_RETURN;
    }
    if (this->state == STATE_ERROR) {
        if (entrySelected(input)) {
            return ApplicationState::SUBSTATE_RETURN;
        }
        return ApplicationState::SUBSTATE_RUNNING;
    }

    if (this->state == STATE_OPEN_ODD1) {
        auto ret = IOSUHAX_FSA_RawOpen(gFSAfd, "/dev/odd01", &(this->oddFd));
        if (ret >= 0) {
            if (this->sectorBuf == nullptr) {
                this->sectorBuf = (void *) memalign(0x100, this->sectorBufSize);
                if (this->sectorBuf == nullptr) {
                    DEBUG_FUNCTION_LINE("ERROR_MALLOC_FAILED");
                    this->setError(ERROR_MALLOC_FAILED);
                    return ApplicationState::SUBSTATE_RUNNING;
                }
            }
            DEBUG_FUNCTION_LINE("Opened /dev/odd01 %d", this->oddFd);
            this->state = STATE_READ_DISC_INFO;
        } else {
            this->state = STATE_PLEASE_INSERT_DISC;
        }
    } else if (this->state == STATE_PLEASE_INSERT_DISC) {
        if (entrySelected(input)) {
            return ApplicationState::SUBSTATE_RETURN;
        }
    } else if (this->state == STATE_READ_DISC_INFO) {
        if (IOSUHAX_FSA_RawRead(gFSAfd, this->sectorBuf, READ_SECTOR_SIZE, 1, 0, this->oddFd) >= 0) {
            this->discId[10] = '\0';
            memcpy(this->discId.data(), sectorBuf, 10);
            if (this->discId[0] == 0) {
                setError(ERROR_NO_DISC_ID);
                return ApplicationState::SUBSTATE_RUNNING;
            }

            this->state = STATE_READ_DISC_INFO_DONE;
            return ApplicationState::SUBSTATE_RUNNING;
        }

        this->setError(ERROR_READ_FIRST_SECTOR);
        return ApplicationState::SUBSTATE_RUNNING;
    } else if (this->state == STATE_READ_DISC_INFO_DONE) {
        this->state = STATE_READ_COMMON_KEY;
    } else if (this->state == STATE_READ_COMMON_KEY) {
        uint8_t opt[0x400];
        IOSUHAX_read_otp(opt, 0x400);
        memcpy(cKey.data(), opt + 0xE0, 0x10);
        this->state = STATE_CREATE_DISC_READER;
    } else if (this->state == STATE_CREATE_DISC_READER) {
        this->discReader = std::make_shared<DiscReaderDiscDrive>();
        if (!discReader->IsReady()) {
            this->setError(ERROR_OPEN_ODD1);
            return SUBSTATE_RUNNING;
        }
        this->state = STATE_PARSE_DISC_HEADER;
    } else if (this->state == STATE_PARSE_DISC_HEADER) {
        auto discHeaderOpt = WiiUDiscHeader::make_unique(discReader);
        if (!discHeaderOpt.has_value()) {
            DEBUG_FUNCTION_LINE("Failed to read DiscHeader");
            this->setError(ERROR_PARSE_DISCHEADER);
            return SUBSTATE_RUNNING;
        }
        this->discHeader = std::move(discHeaderOpt.value());
        this->state = STATE_PROCESS_GM_PARTITIONS;
    } else if (this->state == STATE_PROCESS_GM_PARTITIONS) {
        this->gmPartitionPairs.clear();
        for (auto &partition: discHeader->wiiUContentsInformation->partitions->partitions) {
            auto gmPartition = std::dynamic_pointer_cast<WiiUGMPartition>(partition);
            if (gmPartition != nullptr) {
                auto nusTitleOpt = NUSTitle::loadTitleFromGMPartition(gmPartition, discReader, cKey);
                if (!nusTitleOpt.has_value()) {
                    this->setError(ERROR_FAILED_TO_GET_NUSTITLE);
                    return SUBSTATE_RUNNING;
                }

                this->gmPartitionPairs.emplace_back(gmPartition, nusTitleOpt.value());
            }
        }
        this->state = STATE_CHOOSE_PARTITION_TO_DUMP;
    } else if (this->state == STATE_CHOOSE_PARTITION_TO_DUMP) {
        if (gmPartitionPairs.empty()) {
            if (entrySelected(input)) {
                return SUBSTATE_RETURN;
            }
        }
        proccessMenuNavigationY(input, (int32_t) gmPartitionPairs.size() + 1);
        if (entrySelected(input)) {
            if (selectedOptionY >= (int32_t) gmPartitionPairs.size()) {
                return SUBSTATE_RETURN;
            }
            auto gmPartitionPair = gmPartitionPairs[selectedOptionY];
            if (gmPartitionPair.first != nullptr) {
                this->curPartition = gmPartitionPair.first;
                this->curNUSTitle = gmPartitionPair.second;
                this->dataProvider = this->curNUSTitle->dataProcessor->getDataProvider();
            } else {
                DEBUG_FUNCTION_LINE("Failed to find a GM partition");
                this->setError(ERROR_NO_GM_PARTITION);
                return SUBSTATE_RUNNING;
            }

            this->targetPath = StringTools::strfmt("%swudump/%s/%s", getPathForDevice(targetDevice).c_str(), this->discId, curPartition->getVolumeId().c_str());
            if (!FSUtils::CreateSubfolder(targetPath.c_str())) {
                this->setError(ERROR_CREATE_DIR);
                return SUBSTATE_RUNNING;
            }

            this->state = STATE_DUMP_PARTITION_TMD;
        }
    } else if (this->state == STATE_DUMP_PARTITION_TMD) {
        std::vector<uint8_t> wBuffer;
        if (dataProvider->getRawTMD(wBuffer)) {
            std::string fileName = targetPath + "/" + WUD_TMD_FILENAME;
            if (FSUtils::saveBufferToFile(fileName.c_str(), wBuffer.data(), wBuffer.size()) != (int32_t) wBuffer.size()) {
                this->setError(ERROR_FAILED_WRITE_TMD);
                return SUBSTATE_RUNNING;
            }
            wBuffer.clear();
        }
        this->state = STATE_DUMP_PARTITION_TICKET;
    } else if (this->state == STATE_DUMP_PARTITION_TICKET) {
        std::vector<uint8_t> wBuffer;
        if (dataProvider->getRawTicket(wBuffer)) {
            std::string fileName = targetPath + "/" + WUD_TICKET_FILENAME;
            if (FSUtils::saveBufferToFile(fileName.c_str(), wBuffer.data(), wBuffer.size()) != (int32_t) wBuffer.size()) {
                this->setError(ERROR_FAILED_WRITE_TICKET);
                return SUBSTATE_RUNNING;
            }
            wBuffer.clear();
        } else {
            this->setError(ERROR_FAILED_WRITE_TICKET);
            return SUBSTATE_RUNNING;
        }
        this->state = STATE_DUMP_PARTITION_CERT;
    } else if (this->state == STATE_DUMP_PARTITION_CERT) {
        std::vector<uint8_t> wBuffer;
        if (dataProvider->getRawCert(wBuffer)) {
            std::string fileName = targetPath + "/" + WUD_CERT_FILENAME;
            if (FSUtils::saveBufferToFile(fileName.c_str(), wBuffer.data(), wBuffer.size()) != (int32_t) wBuffer.size()) {
                this->setError(ERROR_FAILED_WRITE_CERT);
                return SUBSTATE_RUNNING;
            }
            wBuffer.clear();
        } else {
            this->setError(ERROR_FAILED_WRITE_TICKET);
            return SUBSTATE_RUNNING;
        }
        this->curContentIndex = 0;
        this->state = STATE_DUMP_PARTITION_CONTENTS;
    } else if (this->state == STATE_DUMP_PARTITION_CONTENTS) {
        // Get current content by index.
        if (curContent == nullptr) {
            auto curContentOpt = curNUSTitle->tmd->getContentByIndex(curContentIndex);
            if (!curContentOpt.has_value()) {
                this->setError(ERROR_FIND_CONTENT_BY_INDEX);
                return SUBSTATE_RUNNING;
            }
            this->curContent = curContentOpt.value();
        }

        // Check if we're done reading the file.
        uint32_t size = ROUNDUP(curContent->encryptedFileSize, 16);
        if (curOffsetInContent >= size) {
            // Close current file
            this->contentFile->close();
            // Free content
            this->contentFile.reset();
            this->curContent.reset();

            curOffsetInContent = 0;

            auto contentListSize = curNUSTitle->tmd->contentList.size();
            if (curContentIndex + 1 < (int32_t) contentListSize) {
                curContentIndex++;
                return SUBSTATE_RUNNING;
            } else {
                this->state = STATE_DUMP_DONE;
                return SUBSTATE_RUNNING;
            }
        }

        // Open the file if needed
        if (this->contentFile == nullptr) {
            char bufApp[32];
            snprintf(bufApp, 31, "%08X.app", curContent->ID);
            std::string appFileName = targetPath + "/" + bufApp;

            this->contentFile = std::make_unique<CFile>(appFileName, CFile::WriteOnly);
            if (!contentFile->isOpen()) {
                this->contentFile.reset();
                this->setError(ERROR_FAILED_CREATE_FILE);
                return SUBSTATE_RUNNING;
            }

            // dump .h3 file as well
            std::vector<uint8_t> h3Data;
            if (dataProvider->getContentH3Hash(this->curContent, h3Data)) {
                snprintf(bufApp, 31, "%08X.h3", curContent->ID);
                std::string h3FileName = targetPath + "/" + bufApp;
                if (FSUtils::saveBufferToFile(h3FileName.c_str(), h3Data.data(), h3Data.size()) != (int32_t) h3Data.size()) {
                    this->setError(ERROR_FAILED_WRITE_H3);
                    return SUBSTATE_RUNNING;
                }
            }
        }

        // alloc readBuffer if needed
        if (this->readBuffer == nullptr) {
            readBuffer = (uint8_t *) malloc(READ_BUFFER_SIZE);
            if (readBuffer == nullptr) {
                this->setError(ERROR_MALLOC_FAILED);
                return SUBSTATE_RUNNING;
            }
        }

        uint32_t toRead = size - curOffsetInContent;
        if (toRead > READ_BUFFER_SIZE) {
            toRead = READ_BUFFER_SIZE;
        }

        if (!dataProvider->readRawContent(curContent, readBuffer, curOffsetInContent, toRead)) {
            this->setError(ERROR_READ_CONTENT);
            return SUBSTATE_RUNNING;
        }

        if (contentFile->write((const uint8_t *) readBuffer, toRead) != (int32_t) toRead) {
            this->setError(ERROR_WRITE_CONTENT);
            return SUBSTATE_RUNNING;
        }
        curOffsetInContent += toRead;

        // Go on!
        this->state = STATE_DUMP_PARTITION_CONTENTS;
        return ApplicationState::SUBSTATE_RUNNING;
    } else if (state == STATE_DUMP_DONE) {
        if (entrySelected(input)) {
            return ApplicationState::SUBSTATE_RETURN;
        }
    }

    return ApplicationState::SUBSTATE_RUNNING;
}

void GMPartitionsDumperState::setError(GMPartitionsDumperState::eErrorState err) {
    this->state = STATE_ERROR;
    this->errorState = err;
    //OSEnableHomeButtonMenu(true);
}

std::string GMPartitionsDumperState::getPathForDevice(eDumpTarget target) const {
    if (target == TARGET_NTFS){
        return "ntfs0:/";
    }
    return "fs:/vol/external01/";
}

std::string GMPartitionsDumperState::ErrorMessage() const {
    if (this->errorState == ERROR_MALLOC_FAILED) {
        return "ERROR_MALLOC_FAILED";
    }
    if (this->errorState == ERROR_NO_DISC_ID) {
        return "ERROR_NO_DISC_ID";
    }
    if (this->errorState == ERROR_READ_FIRST_SECTOR) {
        return "ERROR_READ_FIRST_SECTOR";
    }
    if (this->errorState == ERROR_OPEN_ODD1) {
        return "ERROR_OPEN_ODD1";
    }
    if (this->errorState == ERROR_PARSE_DISCHEADER) {
        return "ERROR_PARSE_DISCHEADER";
    }
    if (this->errorState == ERROR_NO_GM_PARTITION) {
        return "ERROR_NO_GM_PARTITION";
    }
    if (this->errorState == ERROR_CREATE_DIR) {
        return "ERROR_CREATE_DIR";
    }
    if (this->errorState == ERROR_FAILED_TO_GET_NUSTITLE) {
        return "ERROR_FAILED_TO_GET_NUSTITLE";
    }
    if (this->errorState == ERROR_FAILED_WRITE_TMD) {
        return "ERROR_FAILED_WRITE_TMD";
    }
    if (this->errorState == ERROR_FAILED_WRITE_TICKET) {
        return "ERROR_FAILED_WRITE_TICKET";
    }
    if (this->errorState == ERROR_FAILED_WRITE_CERT) {
        return "ERROR_FAILED_WRITE_CERT";
    }
    if (this->errorState == ERROR_FIND_CONTENT_BY_INDEX) {
        return "ERROR_FIND_CONTENT_BY_INDEX";
    }
    if (this->errorState == ERROR_FAILED_CREATE_FILE) {
        return "ERROR_FAILED_CREATE_FILE";
    }
    if (this->errorState == ERROR_FAILED_WRITE_H3) {
        return "ERROR_FAILED_WRITE_H3";
    }
    if (this->errorState == ERROR_READ_CONTENT) {
        return "ERROR_READ_CONTENT";
    }
    if (this->errorState == ERROR_WRITE_CONTENT) {
        return "ERROR_WRITE_CONTENT";
    }
    if (this->errorState == ERROR_MALLOC_FAILED) {
        return "ERROR_MALLOC_FAILED";
    }
    return "UNKNOWN_ERROR";
}

std::string GMPartitionsDumperState::ErrorDescription() const {
    if (this->errorState == ERROR_MALLOC_FAILED) {
        return "ERROR_MALLOC_FAILED";
    }
    if (this->errorState == ERROR_NO_DISC_ID) {
        return "ERROR_NO_DISC_ID";
    }
    if (this->errorState == ERROR_READ_FIRST_SECTOR) {
        return "ERROR_READ_FIRST_SECTOR";
    }
    if (this->errorState == ERROR_OPEN_ODD1) {
        return "ERROR_OPEN_ODD1";
    }
    if (this->errorState == ERROR_PARSE_DISCHEADER) {
        return "ERROR_PARSE_DISCHEADER";
    }
    if (this->errorState == ERROR_NO_GM_PARTITION) {
        return "ERROR_NO_GM_PARTITION";
    }
    if (this->errorState == ERROR_FAILED_TO_GET_NUSTITLE) {
        return "ERROR_FAILED_TO_GET_NUSTITLE";
    }
    if (this->errorState == ERROR_FAILED_WRITE_TMD) {
        return "ERROR_FAILED_WRITE_TMD";
    }
    if (this->errorState == ERROR_FAILED_WRITE_TICKET) {
        return "ERROR_FAILED_WRITE_TICKET";
    }
    if (this->errorState == ERROR_FAILED_WRITE_CERT) {
        return "ERROR_FAILED_WRITE_CERT";
    }
    if (this->errorState == ERROR_FIND_CONTENT_BY_INDEX) {
        return "ERROR_FIND_CONTENT_BY_INDEX";
    }
    if (this->errorState == ERROR_FAILED_CREATE_FILE) {
        return "ERROR_FAILED_CREATE_FILE";
    }
    if (this->errorState == ERROR_FAILED_WRITE_H3) {
        return "ERROR_FAILED_WRITE_H3";
    }
    if (this->errorState == ERROR_READ_CONTENT) {
        return "ERROR_READ_CONTENT";
    }
    if (this->errorState == ERROR_WRITE_CONTENT) {
        return "ERROR_WRITE_CONTENT";
    }
    if (this->errorState == ERROR_MALLOC_FAILED) {
        return "ERROR_MALLOC_FAILED";
    }
    return "UNKNOWN_ERROR";
}


