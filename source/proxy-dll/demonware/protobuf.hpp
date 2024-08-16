#include <cstdint>
#include <string>

enum eWireType
{
    WIRETYPE_VARINT = 0,
    WIRETYPE_64BIT = 1,
    WIRETYPE_STRING = 2,
    WIRETYPE_32BIT = 5,
    WIRETYPE_INVALID = -1
};

class bdProtobufHelper {
public:
    std::string buffer;
    size_t length = 0;

    bdProtobufHelper() = default;
    ~bdProtobufHelper() = default;

    bool encodeString(const char* value, uint32_t len);
    bool encodeTag(uint32_t tagId, eWireType wireType);
    bool writeInt64(uint32_t tag, int64_t value);
    bool writeUInt64(uint32_t tag, uint64_t value);
    bool writeInt32(uint32_t tag, int32_t value);
    bool writeUInt32(uint32_t tag, uint32_t value);
    bool writeString(uint32_t tag, const char* value, uint32_t size);
    bool writeBlob(uint32_t tag, void* data, uint32_t size);

private:
    bool encodeVarInt(uint64_t value);
    bool encodeVarInt(int64_t value);
};