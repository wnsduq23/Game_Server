#pragma once
#include <basetsd.h>

class SerializeBuffer;

SerializeBuffer& operator << (SerializeBuffer& out, INT8 iValue);
SerializeBuffer& operator << (SerializeBuffer& out, INT16 iValue);
SerializeBuffer& operator << (SerializeBuffer& out, INT32 iValue);
SerializeBuffer& operator << (SerializeBuffer& out, INT64 iValue);

SerializeBuffer& operator << (SerializeBuffer& out, UINT8 uchValue);
SerializeBuffer& operator << (SerializeBuffer& out, UINT16 ushValue);
SerializeBuffer& operator << (SerializeBuffer& out, UINT32 uiValue);
SerializeBuffer& operator << (SerializeBuffer& out, UINT64 ullValue);

SerializeBuffer& operator >> (SerializeBuffer& out, INT8& iValue);
SerializeBuffer& operator >> (SerializeBuffer& out, INT16& iValue);
SerializeBuffer& operator >> (SerializeBuffer& out, INT32& iValue);
SerializeBuffer& operator >> (SerializeBuffer& out, INT64& iValue);

SerializeBuffer& operator >> (SerializeBuffer& out, UINT8& uchValue);
SerializeBuffer& operator >> (SerializeBuffer& out, UINT16& ushValue);
SerializeBuffer& operator >> (SerializeBuffer& out, UINT32& uiValue);
SerializeBuffer& operator >> (SerializeBuffer& out, UINT64& ullValue);
