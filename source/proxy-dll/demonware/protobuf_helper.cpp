#include <std_include.hpp>
#include "protobuf_helper.hpp"
#include "ida_defs.h"

bdProtobufHelper::bdProtobufHelper() {}
bdProtobufHelper::~bdProtobufHelper() {}

/*  Yes we unlocked next level of copy-pasta; someone bring the trophy :) */

bool bdProtobufHelper::encodeVarInt(uint64_t value)
{
    __int64 v2; // rbx
    unsigned __int64 v4; // rax
    char v5; // cl
    char valuea[16]; // [rsp+20h] [rbp-28h] BYREF

    v2 = 0i64;
    if (value)
    {
        do
        {
            if ((unsigned int)v2 >= 0xA)
                break;
            v4 = value;
            v5 = value | 0x80;
            value >>= 7;
            valuea[v2] = v5;
            v2 = (unsigned int)(v2 + 1);
        } while (v4 >= 0x80);
        valuea[(unsigned int)(v2 - 1)] &= 0x7Fu;
    }
    else
    {
        valuea[0] = 0;
        LODWORD(v2) = 1;
    }
    //return bdStructSerializationOutputStream::write(stream, valuea, v2) == (_DWORD)v2;
    this->buffer.append(std::string(valuea, v2));
    this->length += v2;

    return true;
}

bool bdProtobufHelper::encodeVarInt(int64_t value)
{
    unsigned __int64 v2; // rax

    v2 = 2 * value;
    if (value < 0)
        v2 = ~v2;
    return bdProtobufHelper::encodeVarInt(v2);
}

bool bdProtobufHelper::encodeString(const char* value, uint32_t len)
{
    bool result; // al

    result = bdProtobufHelper::encodeVarInt(static_cast<uint64_t>(len));
    if (result)
    {
        this->buffer.append(std::string(value, len));
        this->length += len;
    }
    //result = bdStructSerializationOutputStream::write(stream, value, len) == len;

    return result;
}

bool bdProtobufHelper::encodeTag(uint32_t tagId, eWireType wireType)
{
    unsigned __int64 tag = (int)wireType | (8i64 * tagId);
    return bdProtobufHelper::encodeVarInt(tag);
}

bool bdProtobufHelper::writeInt64(uint32_t tag, int64_t value)
{
    return bdProtobufHelper::encodeTag(tag, WIRETYPE_VARINT) && bdProtobufHelper::encodeVarInt(value);
}

bool bdProtobufHelper::writeUInt64(uint32_t tag, uint64_t value)
{
    return bdProtobufHelper::encodeTag(tag, WIRETYPE_VARINT) && bdProtobufHelper::encodeVarInt(value);
}

bool bdProtobufHelper::writeInt32(uint32_t tag, int32_t value)
{
    return bdProtobufHelper::encodeTag(tag, WIRETYPE_VARINT) && bdProtobufHelper::encodeVarInt(static_cast<int64_t>(value));
}

bool bdProtobufHelper::writeUInt32(uint32_t tag, uint32_t value)
{
    return bdProtobufHelper::encodeTag(tag, WIRETYPE_VARINT) && bdProtobufHelper::encodeVarInt(static_cast<uint64_t>(value));
}

bool bdProtobufHelper::writeString(uint32_t tag, const char* value, uint32_t size)
{
    unsigned int v5; // edi
    const void* v8; // rax

    v5 = size;
    v8 = memchr(value, 0, size);
    if (v8)
        v5 = (_DWORD)v8 - (_DWORD)value;
    return bdProtobufHelper::encodeTag(tag, WIRETYPE_STRING) && bdProtobufHelper::encodeString(value, v5);
}

bool bdProtobufHelper::writeBlob(uint32_t tag, void* buffer, uint32_t size)
{
    return bdProtobufHelper::encodeTag(tag, WIRETYPE_STRING) && bdProtobufHelper::encodeString(reinterpret_cast<const char*>(buffer), size);
}