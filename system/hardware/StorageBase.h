#pragma once

#include "PLAMIO.h"

namespace PLAMIO {

class StorageBaseFile : public Storage::File {
protected:
    enum class OpenMode : uint8_t
    {
        CLOSED,
        READ,
        WRITE
    };
    OpenMode mode = OpenMode::CLOSED;

public:
    virtual uint32_t write(const void* buffer, uint32_t bytes) = 0;
    virtual bool closeWrite() = 0;
    ~StorageBaseFile() override = default;
};

class StorageBase : public Storage
{
private:
    static constexpr uint8_t GAME_ID_MAX_LENGTH = 32;
    static constexpr uint32_t TEXT_READ_BUFFER_SIZE = 256;

protected:
    static bool isValidGameId(const char* gameId);
    virtual StorageBaseFile* openWrite(const char* gameId, const char* fileName) = 0;

public:
    using BinaryFileWriterHandler = bool(*)(StorageBaseFile& file, void* arg);

    virtual ~StorageBase() = default;
    virtual const char* getName() const = 0;
    virtual bool begin() = 0; 
    virtual void end() = 0; 

    virtual File* openRead(const char* gameId, const char* fileName) = 0;
    bool readUserFile(const char* gameId, const char* fileName, Storage::UserFileLineReaderHandler handler, void* arg) override;
    bool writeUserFile(const char* gameId, const char* fileName, Storage::UserFileLineWriterHandler writer, void* arg) override;
    bool writeUserFile(const char* gameId, const char* fileName, const char* data) override;

    // UserFile領域にファイルが存在するか確認する。
    bool userFileExists(const char* gameId, const char* fileName);

    // UserFile領域へ任意のバイナリデータを書き込む。
    // ファイルのopen/closeはStorageBaseが管理する。
    bool writeBinaryFile(const char* gameId, const char* fileName, BinaryFileWriterHandler writer, void* arg);
};

} // namespace
