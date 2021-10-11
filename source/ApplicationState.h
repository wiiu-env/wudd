#pragma once

#include <map>
#include <string>
#include <optional>
#include <queue>
#include <coreinit/time.h>
#include "input/Input.h"
#include "fs/CFile.hpp"

#define SECTOR_SIZE 0x8000
#define READ_SECTOR_SIZE SECTOR_SIZE
#define READ_NUM_SECTORS 128
#define WRITE_BUFFER_NUM_SECTORS 128
#define WUD_FILE_SIZE 0x5D3A00000L


typedef struct {
    unsigned int magic0;
    unsigned int magic1;
    unsigned int sectorSize;
    unsigned long long uncompressedSize;
    unsigned int flags;
} wuxHeader_t;

#define WUX_MAGIC_0    0x57555830
#define WUX_MAGIC_1    swap_uint32(0x1099d02e)

class ApplicationState {
public:

    enum eDumpTargetFormat {
        DUMP_AS_WUX,
        DUMP_AS_WUD,
        DUMP_AS_APP,
    };


    enum eErrorState {
        ERROR_NONE,
        ERROR_IOSUHAX_FAILED,
        ERROR_OPEN_ODD1,
        ERROR_READ_FIRST_SECTOR,
        ERROR_FILE_OPEN_FAILED,
        ERROR_MALLOC_FAILED,
        ERROR_NO_DISC_ID,
        ERROR_SECTOR_SIZE,
        ERROR_MAGIC_NUMBER_WRONG,
        ERROR_WRITE_FAILED,
    };

    enum eGameState {
        STATE_ERROR,
        STATE_WELCOME_SCREEN,
        STATE_CHOOSE_TARGET,
        STATE_OPEN_ODD1,
        STATE_PLEASE_INSERT_DISC,
        STATE_DUMP_APP_FILES,
        STATE_DUMP_APP_FILES_DONE,
        STATE_READ_DISC_INFO,
        STATE_READ_DISC_INFO_DONE,
        STATE_DUMP_DISC_KEY,
        STATE_DUMP_DISC_START,
        STATE_DUMP_DISC_DONE,
        STATE_WAIT_USER_ERROR_CONFIRM,
        STATE_DUMP_DISC,
    };

    ApplicationState();

    ~ApplicationState();

    void setError(eErrorState error);

    void render();

    void update(Input *input);

    std::string ErrorMessage();

    std::string ErrorDescription();

    int selectedOption;

    static void printFooter();

    void proccessMenuNavigation(Input *input, int maxOptionValue);

    static bool entrySelected(Input *input);

private:
    static void printHeader();

    CFile log;
    eGameState state;
    eDumpTargetFormat dumpFormat;
    eErrorState error = ERROR_NONE;
    std::string target = "fs:/vol/external01/";
    int oddFd = -1;
    int retryCount = 0;
    void *sectorBuf = nullptr;
    int sectorBufSize = READ_NUM_SECTORS * READ_SECTOR_SIZE;
    char discId[11];
    uint64_t currentSector = 0;
    std::vector<uint64_t> skippedSectors;
    int readResult = 0;

    [[nodiscard]] bool writeDataToFile(void *buffer, int numberOfSection);

    uint64_t totalSectorCount = 0;

    std::map<std::string, int32_t> hashMap;
    CFile *fileHandle;
    OSTime startTime;
    void *writeBuffer = nullptr;
    uint32_t writeBufferPos = 0;
    uint32_t writeBufferSize = 0;

    [[nodiscard]] bool writeCached(uint32_t addr, uint32_t writeSize);

    void clearWriteCache();

    [[nodiscard]] bool flushWriteCache();

    uint32_t readSectors = 0;
    uint64_t sectorTableStart = 0;
    void *sectorIndexTable = nullptr;
    uint64_t sectorTableEnd = 0;

    void writeSectorIndexTable();

    void *emptySector = nullptr;
    bool doWUX = false;
    uint32_t writtenSector = 0;
    bool autoSkip = false;

    void dumpAppFiles();

    static void printDumpState(const char *fmt, ...);
};