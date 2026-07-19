#include "ScreenShot.h"

#include <new>
#include <stdio.h>
#include <cstring>

using namespace PLAMIO;

namespace
{
    constexpr uint32_t BMP_FILE_HEADER_SIZE = 14;
    constexpr uint32_t BMP_INFO_HEADER_SIZE = 40;
    constexpr uint32_t BMP_PIXEL_DATA_OFFSET = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;

    constexpr uint32_t align4(uint32_t value)
    {
        return (value + 3U) & ~3U;
    }
}

bool ScreenShot::save(GraphicsBase& graphics, StorageBase& storage)
{
    if (!storage.isAvailable()) return false;
    if (graphics.getScreenWidth() == 0 || graphics.getScreenHeight() == 0) return false;

    char fileName[32];
    if (!makeNextFileName(storage, fileName, sizeof(fileName))) return false;

    uint8_t* writeBuffer = new (std::nothrow) uint8_t[WRITE_BUFFER_SIZE];
    if (writeBuffer == nullptr) return false;

    WriteContext context = {&graphics, writeBuffer, 0};
    const bool result = storage.writeBinaryFile(STORAGE_ID, fileName, &ScreenShot::writeBmp, &context);

    delete[] writeBuffer;
    return result;
}

bool ScreenShot::makeNextFileName(StorageBase& storage, char* outFileName, size_t outSize)
{
    if (outFileName == nullptr || outSize == 0) return false;

    for (uint32_t sequence = 1; sequence <= MAX_SEQUENCE; ++sequence)
    {
        const int length = snprintf(
            outFileName,
            outSize,
            "screenshot_%06lu.bmp",
            static_cast<unsigned long>(sequence));

        if (length <= 0 || static_cast<size_t>(length) >= outSize) return false;
        if (!storage.userFileExists(STORAGE_ID, outFileName)) return true;
    }

    return false;
}

bool ScreenShot::writeBmp(StorageBaseFile& file, void* arg)
{
    if (arg == nullptr) return false;

    WriteContext* context = static_cast<WriteContext*>(arg);
    if (context->graphics == nullptr) return false;

    GraphicsBase& graphics = *context->graphics;
    const uint16_t width = graphics.getScreenWidth();
    const uint16_t height = graphics.getScreenHeight();
    if (width == 0 || height == 0) return false;

    const uint32_t rawRowSize = static_cast<uint32_t>(width) * 3U;
    const uint32_t storedRowSize = align4(rawRowSize);
    const uint32_t paddingSize = storedRowSize - rawRowSize;
    const uint32_t imageSize = storedRowSize * static_cast<uint32_t>(height);
    const uint32_t fileSize = BMP_PIXEL_DATA_OFFSET + imageSize;

    const uint8_t signature[2] = {'B', 'M'};
    if (!writeBytes(file, *context, signature, sizeof(signature))) return false;
    if (!writeUInt32LE(file, *context, fileSize)) return false;
    if (!writeUInt16LE(file, *context, 0)) return false;
    if (!writeUInt16LE(file, *context, 0)) return false;
    if (!writeUInt32LE(file, *context, BMP_PIXEL_DATA_OFFSET)) return false;

    if (!writeUInt32LE(file, *context, BMP_INFO_HEADER_SIZE)) return false;
    if (!writeInt32LE(file, *context, static_cast<int32_t>(width))) return false;
    if (!writeInt32LE(file, *context, static_cast<int32_t>(height))) return false;
    if (!writeUInt16LE(file, *context, 1)) return false;
    if (!writeUInt16LE(file, *context, 24)) return false;
    if (!writeUInt32LE(file, *context, 0)) return false;
    if (!writeUInt32LE(file, *context, imageSize)) return false;
    if (!writeInt32LE(file, *context, 0)) return false;
    if (!writeInt32LE(file, *context, 0)) return false;
    if (!writeUInt32LE(file, *context, 0)) return false;
    if (!writeUInt32LE(file, *context, 0)) return false;

    uint16_t* rgb565Line = new (std::nothrow) uint16_t[width];
    uint8_t* bgrLine = new (std::nothrow) uint8_t[rawRowSize];
    if (rgb565Line == nullptr || bgrLine == nullptr)
    {
        delete[] rgb565Line;
        delete[] bgrLine;
        return false;
    }

    bool ok = true;
    const uint8_t padding[3] = {0, 0, 0};

    for (int32_t y = static_cast<int32_t>(height) - 1; y >= 0 && ok; --y)
    {
        if (!graphics.readScreenLine(static_cast<uint16_t>(y), rgb565Line, width))
        {
            ok = false;
            break;
        }

        for (uint16_t x = 0; x < width; ++x)
        {
            const uint16_t color = rgb565Line[x];
            const uint8_t r5 = static_cast<uint8_t>((color >> 11) & 0x1F);
            const uint8_t g6 = static_cast<uint8_t>((color >> 5) & 0x3F);
            const uint8_t b5 = static_cast<uint8_t>(color & 0x1F);
            const uint32_t index = static_cast<uint32_t>(x) * 3U;

            bgrLine[index + 0] = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
            bgrLine[index + 1] = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
            bgrLine[index + 2] = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
        }

        if (!writeBytes(file, *context, bgrLine, rawRowSize)) ok = false;
        if (ok && paddingSize > 0 && !writeBytes(file, *context, padding, paddingSize)) ok = false;
    }

    if (ok) ok = flushWriteBuffer(file, *context);

    delete[] rgb565Line;
    delete[] bgrLine;
    context->writeBufferUsed = 0;
    return ok;
}

bool ScreenShot::flushWriteBuffer(StorageBaseFile& file, WriteContext& context)
{
    if (context.writeBufferUsed == 0) return true;

    const uint32_t written = file.write(context.writeBuffer, context.writeBufferUsed);
    if (written != context.writeBufferUsed) return false;

    context.writeBufferUsed = 0;
    return true;
}

bool ScreenShot::writeBytes(StorageBaseFile& file, WriteContext& context, const void* data, uint32_t size)
{
    if (data == nullptr && size > 0) return false;
    if (context.writeBuffer == nullptr) return false;

    const uint8_t* source = static_cast<const uint8_t*>(data);
    uint32_t remaining = size;

    while (remaining > 0)
    {
        const uint32_t freeBytes = WRITE_BUFFER_SIZE - context.writeBufferUsed;
        const uint32_t copyBytes = remaining < freeBytes ? remaining : freeBytes;

        memcpy(context.writeBuffer + context.writeBufferUsed, source, copyBytes);
        context.writeBufferUsed += copyBytes;
        source += copyBytes;
        remaining -= copyBytes;

        if (context.writeBufferUsed == WRITE_BUFFER_SIZE && !flushWriteBuffer(file, context))
        {
            return false;
        }
    }

    return true;
}

bool ScreenShot::writeUInt16LE(StorageBaseFile& file, WriteContext& context, uint16_t value)
{
    const uint8_t bytes[2] = {
        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF)
    };
    return writeBytes(file, context, bytes, sizeof(bytes));
}

bool ScreenShot::writeUInt32LE(StorageBaseFile& file, WriteContext& context, uint32_t value)
{
    const uint8_t bytes[4] = {
        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 24) & 0xFF)
    };
    return writeBytes(file, context, bytes, sizeof(bytes));
}

bool ScreenShot::writeInt32LE(StorageBaseFile& file, WriteContext& context, int32_t value)
{
    return writeUInt32LE(file, context, static_cast<uint32_t>(value));
}
