#pragma once

#include "GraphicsBase.h"
#include "StorageBase.h"

namespace PLAMIO {

class ScreenShot
{
private:
    static constexpr const char* STORAGE_ID = "screenshots";
    static constexpr uint32_t MAX_SEQUENCE = 999999;

    static constexpr uint32_t WRITE_BUFFER_SIZE = 4096;

    struct WriteContext
    {
        GraphicsBase* graphics;
        uint8_t* writeBuffer;
        uint32_t writeBufferUsed;
    };

    static bool writeBmp(StorageBaseFile& file, void* arg);
    static bool flushWriteBuffer(StorageBaseFile& file, WriteContext& context);
    static bool writeBytes(StorageBaseFile& file, WriteContext& context, const void* data, uint32_t size);
    static bool writeUInt16LE(StorageBaseFile& file, WriteContext& context, uint16_t value);
    static bool writeUInt32LE(StorageBaseFile& file, WriteContext& context, uint32_t value);
    static bool writeInt32LE(StorageBaseFile& file, WriteContext& context, int32_t value);
    static bool makeNextFileName(StorageBase& storage, char* outFileName, size_t outSize);

public:
    static bool save(GraphicsBase& graphics, StorageBase& storage);
};

} // namespace PLAMIO
