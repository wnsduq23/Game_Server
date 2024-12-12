/*#include "Overloading.h"
#include "SerializeBuffer.h"
#include <windows.h>
#include <stdio.h>

SerializeBuffer& operator << (SerializeBuffer& out, INT8 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator << (SerializeBuffer& out, INT16 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator << (SerializeBuffer& out, INT32 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator << (SerializeBuffer& out, INT64 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator << (SerializeBuffer& out, UINT8 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator << (SerializeBuffer& out, UINT16 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator << (SerializeBuffer& out, UINT32 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator << (SerializeBuffer& out, UINT64 iValue)
{
    if (out._iBufferSize - out._iWritePos < sizeof(iValue))
        out.Resize(out._iBufferSize * 1.5f);

    memcpy_s(&out._chpBuffer[out._iWritePos],
        out._iBufferSize - out._iWritePos,
        &iValue, sizeof(iValue));

    out._iWritePos += sizeof(iValue);
    return out;
}

SerializeBuffer& operator >> (SerializeBuffer& out, INT8& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(INT8))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(INT8), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(INT8),
        &out._chpBuffer[out._iReadPos], sizeof(INT8));

    out._iReadPos += sizeof(INT8);
    return out;
}

SerializeBuffer& operator >> (SerializeBuffer& out, INT16& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(INT16))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(INT16), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(INT16),
        &out._chpBuffer[out._iReadPos], sizeof(INT16));

    out._iReadPos += sizeof(INT16);
    return out;
}

SerializeBuffer& operator >> (SerializeBuffer& out, INT32& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(INT32))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(INT32), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(INT32),
        &out._chpBuffer[out._iReadPos], sizeof(INT32));

    out._iReadPos += sizeof(INT32);
    return out;
}

SerializeBuffer& operator >> (SerializeBuffer& out, INT64& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(INT64))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(INT64), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(INT64),
        &out._chpBuffer[out._iReadPos], sizeof(INT64));

    out._iReadPos += sizeof(INT64);
    return out;
}


SerializeBuffer& operator >> (SerializeBuffer& out, UINT8& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(UINT8))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(UINT8), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(UINT8),
        &out._chpBuffer[out._iReadPos], sizeof(UINT8));

    out._iReadPos += sizeof(UINT8);
    return out;
}

SerializeBuffer& operator >> (SerializeBuffer& out, UINT16& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(UINT16))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(UINT16), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(UINT16),
        &out._chpBuffer[out._iReadPos], sizeof(UINT16));

    out._iReadPos += sizeof(UINT16);
    return out;
}

SerializeBuffer& operator >> (SerializeBuffer& out, UINT32& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(UINT32))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(int), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(UINT32),
        &out._chpBuffer[out._iReadPos], sizeof(UINT32));

    out._iReadPos += sizeof(UINT32);
    return out;
}

SerializeBuffer& operator >> (SerializeBuffer& out, UINT64& iValue)
{
    if (out._iWritePos - out._iReadPos < sizeof(UINT64))
    {
        printf("Used Size(%d) < Requested Size(%llu)!: %s %d\n",
            out._iWritePos - out._iReadPos, sizeof(UINT64), __func__, __LINE__);
        return out;
    }

    memcpy_s(&iValue, sizeof(UINT64),
        &out._chpBuffer[out._iReadPos], sizeof(UINT64));

    out._iReadPos += sizeof(UINT64);
    return out;
}*/