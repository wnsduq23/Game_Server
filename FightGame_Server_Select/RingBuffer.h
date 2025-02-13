#pragma once
#define DEFAULT_BUF_SIZE (32767 + 1)
#define MAX_BUF_SIZE (65535 + 1)

/*====================================================================

    <Ring Buffer>

    readPos는 비어있는 공간을,
    writePos는 마지막으로 넣은 공간을 가리킨다
    따라서 readPos == writePos는 버퍼가 비어있음을 의미하고
    버퍼가 찼을 때는 (readPos + 1)%_bufferSize == writePos 가 된다.

======================================================================*/
class RingBuffer
{
public:
    RingBuffer(void);
    RingBuffer(int iBufferSize);
    ~RingBuffer(void);

    int GetBufferSize(void);
    int GetUseSize(void);
    int GetFreeSize(void);
    int DirectEnqueueSize(void);
    int DirectDequeueSize(void);

    int Enqueue(char* chpData, int iSize);
    int Dequeue(char* chpData, int iSize);
    int Peek(char* chpDest, int iSize);
    void ClearBuffer(void);
    bool Resize(int iSize);

    int MoveReadPos(int iSize);
    int MoveWritePos(int iSize);
    char* GetReadPtr(void) { return &_chpBuffer[(_iReadPos + 1) % _iBufferSize]; }
    char* GetWritePtr(void) { return &_chpBuffer[(_iWritePos + 1) % _iBufferSize]; }

    // For Debug
    void GetBufferDataForDebug();

private:
    char* _chpBuffer;
    int _iBufferSize;

    int _iUsedSize = 0;
    int _iFreeSize = 0;

    int _iReadPos = 0;
    int _iWritePos = 0;
};

