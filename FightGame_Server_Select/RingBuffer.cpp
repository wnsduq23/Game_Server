#include "RingBuffer.h"
#include <iostream>

//#define RINGBUFFER_DEBUG
//#define ENQ_DEQ_DEBUG

RingBuffer::RingBuffer(void) : _iBufferSize(DEFAULT_BUF_SIZE), _iFreeSize(DEFAULT_BUF_SIZE - 1)
{
    _chpBuffer = new char[_iBufferSize]();
}

RingBuffer::RingBuffer(int bufferSize) : _iBufferSize(bufferSize), _iFreeSize(bufferSize - 1)
{
    _chpBuffer = new char[_iBufferSize]();
}

RingBuffer::~RingBuffer(void)
{
    delete[] _chpBuffer;
}

int RingBuffer::GetBufferSize(void)
{
    return _iBufferSize;
}

int RingBuffer::GetUseSize(void)
{
#ifdef RINGBUFFER_DEBUG
    int useSize = 0;

    if (_iWritePos > _iReadPos)
    {
        useSize = _iWritePos - _iReadPos;
    }
    else if (_iWritePos < _iReadPos)
    {
        useSize = _iBufferSize - _iReadPos + _iWritePos;
    }

    if (useSize != _iUsedSize)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        printf("\n\n");
        printf("Buffer Size: %d\n", _iBufferSize);
        printf("Read: %d\n", _iReadPos);
        printf("Write: %d\n", _iWritePos);
        printf("Real Use Size: %d\n", _iUsedSize);
        printf("Real Free Size: %d\n", _iFreeSize);
        printf("Calculated Use Size: %d\n", useSize);
        printf("\n");
        return -1;
    }

#endif
    return _iUsedSize;
}

int RingBuffer::GetFreeSize(void)
{
#ifdef RINGBUFFER_DEBUG

    int freeSize = _iBufferSize - 1;

    if (_iWritePos > _iReadPos)
    {
        freeSize = _iBufferSize - _iWritePos + _iReadPos - 1;
    }
    else if (_iWritePos < _iReadPos)
    {
        freeSize = _iReadPos - _iWritePos - 1;
    }

    if (freeSize != _iFreeSize)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        printf("\n\n");
        printf("Buffer Size: %d\n", _iBufferSize);
        printf("Read: %d\n", _iReadPos);
        printf("Write: %d\n", _iWritePos);
        printf("Real Use Size: %d\n", _iUsedSize);
        printf("Real Free Size: %d\n", _iFreeSize);
        printf("Calculated Free Size: %d\n", freeSize);
        printf("\n");
        return -1;
    }
#endif
    return _iFreeSize;
}

int RingBuffer::DirectEnqueueSize(void)
{
    int directEnqueueSize = -1;

    if (_iWritePos >= _iReadPos)
    {
        directEnqueueSize = _iBufferSize - _iWritePos - 1;
    }
    else
    {
        directEnqueueSize = _iReadPos - _iWritePos - 1;
    }

    return directEnqueueSize;
}

int RingBuffer::DirectDequeueSize(void)
{
    int directDequeueSize = -1;

    if (_iWritePos >= _iReadPos)
    {
        directDequeueSize = _iWritePos - _iReadPos;
    }
    else
    {
        directDequeueSize = _iBufferSize - _iReadPos - 1;
    }

    return directDequeueSize;
}


int RingBuffer::Enqueue(char* chpData, int iSize)
{
#ifdef RINGBUFFER_DEBUG
    if (GetFreeSize() < 0 || DirectEnqueueSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif

    if (iSize > GetFreeSize())
    {
        if (!Resize(_iBufferSize + iSize))
        {
            printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
            return -1;
        }
    }

    if (iSize <= DirectEnqueueSize())
    {
        memcpy_s(&_chpBuffer[(_iWritePos + 1) % _iBufferSize], iSize, chpData, iSize);

#ifdef ENQ_DEQ_DEBUG
        printf("\n\n");
        printf("Enqueue===========================\n\n");
        int idx = 0;
        while (idx < iSize)
        {
            printf("0x%X ", chpData[idx]);
            idx++;
        }
        printf("\n");

        idx = 0;
        while (idx < iSize)
        {
            printf("0x%X ", _chpBuffer[(_iWritePos + 1) % _iBufferSize + idx]);
            idx++;
        }
        printf("\n\n===================================");
        printf("\n\n");
#endif
    }
    else
    {
        int size1 = DirectEnqueueSize();
        int size2 = iSize - size1;
        memcpy_s(&_chpBuffer[(_iWritePos + 1) % _iBufferSize], size1, chpData, size1);
        memcpy_s(_chpBuffer, size2, &chpData[size1], size2);

#ifdef ENQ_DEQ_DEBUG
        printf("\n\n");
        printf("Enqueue===========================\n\n");
        int idx = 0;
        while (idx < iSize)
        {
            printf("0x%X ", chpData[idx]);
            idx++;
        }
        printf("\n");

        idx = 0;
        while (idx < size1)
        {
            printf("0x%X ", _chpBuffer[(_iWritePos + 1) % _iBufferSize + idx]);
            idx++;
        }
        printf("=[%d]", (_iWritePos + 1) % _iBufferSize + idx - 1);
        idx = 0;
        while (idx < size2)
        {
            printf("0x%X ", _chpBuffer[idx]);
            idx++;
        }
        printf("\n\n===================================");
        printf("\n\n");
#endif
    }

    _iUsedSize += iSize;
    _iFreeSize -= iSize;
    _iWritePos = (_iWritePos + iSize) % _iBufferSize;

#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif

    return iSize;
}

int RingBuffer::Dequeue(char* chpData, int iSize)
{
#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || DirectDequeueSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif

    if (iSize > GetUseSize())
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }

    if (iSize <= DirectDequeueSize())
    {
        memcpy_s(chpData, iSize, &_chpBuffer[(_iReadPos + 1) % _iBufferSize], iSize);

#ifdef ENQ_DEQ_DEBUG
        printf("\n\n");
        printf("Dequeue===========================\n\n");
        int idx = 0;
        while (idx < iSize)
        {
            printf("0x%X ", _chpBuffer[(_iReadPos + 1) % _iBufferSize + idx]);
            idx++;
        }
        printf("\n\n===================================");
        printf("\n\n");
#endif
    }
    else
    {
        int size1 = DirectDequeueSize();
        int size2 = iSize - size1;
        memcpy_s(chpData, size1, &_chpBuffer[(_iReadPos + 1) % _iBufferSize], size1);
        memcpy_s(&chpData[size1], size2, _chpBuffer, size2);

#ifdef ENQ_DEQ_DEBUG
        printf("\n\n");
        printf("Dequeue===========================\n\n");
        int idx = 0;
        while (idx < size1)
        {
            printf("0x%X ", _chpBuffer[(_iReadPos + 1) % _iBufferSize + idx]);
            idx++;
        }
        printf("=[%d]", (_iReadPos + 1) % _iBufferSize + idx - 1);

        idx = 0;
        while (idx < size2)
        {
            printf("0x%X ", _chpBuffer[idx]);
            idx++;
        }
        printf("\n\n===================================");
        printf("\n\n");
#endif
    }

    _iUsedSize -= iSize;
    _iFreeSize += iSize;
    _iReadPos = (_iReadPos + iSize) % _iBufferSize;

#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif
    return iSize;
}

int RingBuffer::Peek(char* chpDest, int iSize)
{
#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || DirectDequeueSize() < 0)
    {
        printf("Error! Func %s Line %d\n", __func__, __LINE__);
        return -1;
    }
#endif

    if (iSize > GetUseSize())
    {
        printf("Error! Func %s Line %d\n", __func__, __LINE__);
        return -1;
    }

    if (iSize <= DirectDequeueSize())
    {
        memcpy_s(chpDest, iSize, &_chpBuffer[(_iReadPos + 1) % _iBufferSize], iSize);
    }
    else
    {
        int size1 = DirectDequeueSize();
        int size2 = iSize - size1;
        memcpy_s(chpDest, size1, &_chpBuffer[(_iReadPos + 1) % _iBufferSize], size1);
        memcpy_s(&chpDest[size1], size2, _chpBuffer, size2);
    }

    return iSize;
}


void RingBuffer::ClearBuffer(void)
{
    _iReadPos = 0;
    _iWritePos = 0;
    _iUsedSize = 0;
    _iFreeSize = _iBufferSize - 1;
}


bool RingBuffer::Resize(int iSize)
{
    if (iSize > MAX_BUF_SIZE)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return false;
    }

    if (iSize < _iUsedSize)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return false;
    }

    char* newBuffer = new char[iSize]();

    if (_iWritePos > _iReadPos)
    {
        memcpy_s(newBuffer, iSize, &_chpBuffer[(_iReadPos + 1) % _iBufferSize], _iUsedSize);
    }
    else if (_iWritePos < _iReadPos)
    {
        int size1 = _iBufferSize - _iReadPos - 1;
        int size2 = _iWritePos + 1;
        memcpy_s(newBuffer, iSize, &_chpBuffer[(_iReadPos + 1) % _iBufferSize], size1);
        memcpy_s(&newBuffer[size1], iSize - size1, _chpBuffer, size2);
    }

    delete[] _chpBuffer;
    _chpBuffer = newBuffer;
    _iBufferSize = iSize;
    _iFreeSize = _iBufferSize - _iUsedSize - 1;
    _iReadPos = 0;
    _iWritePos = _iUsedSize;

#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return false;
    }
#endif
    return true;
}

int RingBuffer::MoveReadPos(int iSize)
{
#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif

    if (iSize > GetUseSize())
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }

    _iReadPos = (_iReadPos + iSize) % _iBufferSize;
    _iUsedSize -= iSize;
    _iFreeSize += iSize;

#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif
    return iSize;
}

int RingBuffer::MoveWritePos(int iSize)
{
#ifdef RINGBUFFER_DEBUG
    if (GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif

    if (iSize > GetFreeSize())
    {
        if (!Resize(_iBufferSize + iSize))
        {
            printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
            return -1;
        }
    }

    _iWritePos = (_iWritePos + iSize) % _iBufferSize;
    _iUsedSize += iSize;
    _iFreeSize -= iSize;

#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return -1;
    }
#endif
    return iSize;
}

char* RingBuffer::GetReadBufferPtr(void)
{
#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return nullptr;
    }
#endif
    return &_chpBuffer[_iReadPos + 1];
}

char* RingBuffer::GetWriteBufferPtr(void)
{
#ifdef RINGBUFFER_DEBUG
    if (GetUseSize() < 0 || GetFreeSize() < 0)
    {
        printf("Error! Func %s Function %s Line %d\n", __func__, __func__, __LINE__);
        return nullptr;
    }
#endif
    return &_chpBuffer[_iWritePos + 1];
}

void RingBuffer::GetBufferDataForDebug()
{
    printf("\n");
    printf("Buffer Size: %d\n", _iBufferSize);
    printf("Read: %d\n", _iReadPos);
    printf("Write: %d\n", _iWritePos);
    printf("Real Use Size: %d\n", _iUsedSize);
    printf("Real Free Size: %d\n", _iFreeSize);
    printf("Direct Dequeue Size: %d\n", DirectDequeueSize());
    printf("Direct Enqueue Size: %d\n", DirectEnqueueSize());
    printf("\n");
}