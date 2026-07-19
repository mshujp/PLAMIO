#pragma once

#include "StorageBase.h"
#include "ff.h"

namespace PLAMIO {

class StorageSDFile : public StorageBaseFile {
private:
    FIL file = {};
    uint32_t fileSize = 0;

public:
    ~StorageSDFile() override;
    bool openRead(const char* path);
    bool openWrite(const char* path);
    bool isOpen() const override { return mode != OpenMode::CLOSED; }
    uint32_t size() const override { return mode != OpenMode::CLOSED ? fileSize : 0; }
    uint32_t read(void* buffer, uint32_t bytes) override;
    uint32_t write(const void* buffer, uint32_t bytes) override;
    void close() override;
    bool closeWrite() override;
};

class StorageSD : public StorageBase {
private:
    static constexpr char kVolumePath[] = "0:";
    static constexpr uint16_t FAT_PATH_MAX = 128;

    static constexpr char ROOT_DIR[] = "/PLAMIO_Games";

    uint8_t spiHost;
    int8_t pinMiso;
    int8_t pinSck;
    int8_t pinMosi;
    int8_t pinCs;
    uint32_t baudRate;
    bool sdAvailable = false;
    StorageSDFile fileSlot;

    bool ensureDirectory(const char* path);
    bool makeFatPath(const char* path, char* outBuffer, size_t bufferSize) const;
    bool getDataDir(char* outBuffer, uint16_t bufferSize, const char* gameId);
    static bool isValidUserFileName(const char* fileName);
    StorageBaseFile* openWrite(const char* gameId, const char* fileName) override;

public:
    struct Config {
        uint8_t spiHost = 0; // 0=spi0, 1=spi1
        int8_t misoPin = -1;
        int8_t sckPin  = -1;
        int8_t mosiPin = -1;
        int8_t csPin   = -1;
        uint32_t baudRate = 12 * 1000 * 1000;
    };
    
    explicit StorageSD(const Config& config);
    const char* getName() const override { return "SD"; }
    bool begin() override;
    void end() override;
    
    bool isAvailable() const override { return sdAvailable; };
    File* openRead(const char* path) override;
    File* openRead(const char* gameId, const char* fileName) override;
    bool directoryExists(const char* path) override;
    bool fileExists(const char* path) override;
};

} // namespace
