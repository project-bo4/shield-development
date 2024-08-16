#include <std_include.hpp>
#include "protobuf.hpp"

bool bdProtobufHelper::encodeVarInt(uint64_t value) {
    char valuea[16];
    size_t index = 0;

    if (value) {
        while (value >= 0x80 && index < 10) {
            valuea[index++] = static_cast<char>(value | 0x80);
            value >>= 7;
        }
        valuea[index++] = static_cast<char>(value);
    }
    else {
        valuea[index++] = 0;
    }

    this->buffer.append(valuea, index);
    this->length += index;
    return true;
}

bool bdProtobufHelper::encodeVarInt(int64_t value) {
    uint64_t encodedValue = 2 * static_cast<uint64_t>(value);
    if (value < 0) {
        encodedValue = ~encodedValue;
    }
    return encodeVarInt(encodedValue);
}

bool bdProtobufHelper::encodeString(const char* value, uint32_t len) {
    bool result = encodeVarInt(static_cast<uint64_t>(len));
    if (result) {
        this->buffer.append(value, len);
        this->length += len;
    }
    return result;
}

bool bdProtobufHelper::encodeTag(uint32_t tagId, eWireType wireType) {
    uint64_t tag = static_cast<int>(wireType) | (static_cast<uint64_t>(tagId) << 3);
    return encodeVarInt(tag);
}

bool bdProtobufHelper::writeInt64(uint32_t tag, int64_t value) {
    return encodeTag(tag, WIRETYPE_VARINT) && encodeVarInt(value);
}

bool bdProtobufHelper::writeUInt64(uint32_t tag, uint64_t value) {
    return encodeTag(tag, WIRETYPE_VARINT) && encodeVarInt(value);
}

bool bdProtobufHelper::writeInt32(uint32_t tag, int32_t value) {
    return encodeTag(tag, WIRETYPE_VARINT) && encodeVarInt(static_cast<int64_t>(value));
}

bool bdProtobufHelper::writeUInt32(uint32_t tag, uint32_t value) {
    return encodeTag(tag, WIRETYPE_VARINT) && encodeVarInt(static_cast<uint64_t>(value));
}

bool bdProtobufHelper::writeString(uint32_t tag, const char* value, uint32_t size) {
    uint32_t actualSize = size;
    const void* nullPos = memchr(value, 0, size);
    if (nullPos) {
        actualSize = static_cast<uint32_t>(static_cast<const char*>(nullPos) - value);
    }
    return encodeTag(tag, WIRETYPE_STRING) && encodeString(value, actualSize);
}

bool bdProtobufHelper::writeBlob(uint32_t tag, void* data, uint32_t size) {
    return encodeTag(tag, WIRETYPE_STRING) && encodeString(reinterpret_cast<const char*>(data), size);
}
