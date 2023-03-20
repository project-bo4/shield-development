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

class bdProtobufHelper
{
public:
    std::string buffer{};
    uint32_t length{};

    bdProtobufHelper();
    ~bdProtobufHelper();

    bool write(const void* value, uint32_t byteCount);
    bool writeString(uint32_t tag, const char* value, uint32_t size);
    bool writeBlob(uint32_t tag, void* buffer, uint32_t size);
    bool writeInt64(uint32_t tag, int64_t value);
    bool writeUInt64(uint32_t tag, uint64_t value);
    bool writeInt32(uint32_t tag, int32_t value);
    bool writeUInt32(uint32_t tag, uint32_t value);


private:
    bool encodeTag(uint32_t tagId, eWireType wireType);
    bool encodeVarInt(int64_t value);
    bool encodeVarInt(uint64_t value);
    bool encodeString(const char* value, uint32_t len);


};