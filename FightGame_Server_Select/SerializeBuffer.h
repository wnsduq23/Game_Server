#ifndef  __SERIALIZE_BUFFER__
#define  __SERIALIZE_BUFFER__

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <windows.h>

class SerializeBuffer
{
public:

	enum en_BUFFER
	{
		eBUFFER_DEFAULT = 1024,
		eBUFFER_MAX = 4096
	};

	SerializeBuffer();
	SerializeBuffer(int iBufferSize);
	virtual	~SerializeBuffer();

	void Clear(void);
	int Resize(int iBufferSize);
	bool IsEmpty(void) { return (_iWritePos == _iReadPos); }
	int	GetBufferSize(void) { return _iBufferSize; }
	int	GetDataSize(void);

	char* GetWritePtr(void) { return &_chpBuffer[_iWritePos]; }
	char* GetReadPtr(void) { return &_chpBuffer[_iReadPos]; }
	int	MoveWritePos(int iSize);
	int	MoveReadPos(int iSize);

	SerializeBuffer& operator = (SerializeBuffer& clSrSerializePacket);

	SerializeBuffer& operator << (float fValue);
	SerializeBuffer& operator << (double dValue);

	SerializeBuffer& operator << (char chValue);
	SerializeBuffer& operator << (unsigned char byValue);

	SerializeBuffer& operator << (short shValue);
	SerializeBuffer& operator << (unsigned short wValue);

	SerializeBuffer& operator << (int iValue);
	SerializeBuffer& operator << (UINT32 uiValue);
	SerializeBuffer& operator << (long lValue);
	SerializeBuffer& operator << (__int64 iValue);

	//--------------------------------------

	SerializeBuffer& operator >> (float& fValue);
	SerializeBuffer& operator >> (double& dValue);

	SerializeBuffer& operator >> (char& chValue);
	SerializeBuffer& operator >> (BYTE& byValue);

	SerializeBuffer& operator >> (short& shValue);
	SerializeBuffer& operator >> (WORD& wValue);

	SerializeBuffer& operator >> (int& iValue);
	SerializeBuffer& operator >> (DWORD& dwValue);
	SerializeBuffer& operator >> (__int64& iValue);

	int	GetData(char* chpDest, int iSize);
	int	PutData(char* chpSrc, int iSrcSize);

protected:
	int	_iBufferSize;
	int	_iDataSize;

private:
	char* _chpBuffer;
	int _iReadPos;
	int _iWritePos;

};

#endif
