#include "SetSCPacket.h"
#include "SystemLog.h"
#include "main.h"
#include <tchar.h>

void SetSCPacket_HEADER(
	stPACKET_HEADER& header,
	BYTE Size, BYTE Type)
{
	header.code = (BYTE)dfPACKET_HEADER_CODE;
	header.payload_size = Size;
	header.action_type = Type;
}

int SetSCPacket_CREATE_MY_CHARACTER(
	SerializePacket* pPacket,
	DWORD ID, BYTE Direction, short X, short Y, char HP) // headDirection
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_CREATE_MY_CHARACTER);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;
	*pPacket << Direction;
	*pPacket << X;
	*pPacket << Y;
	*pPacket << HP;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_CREATE_OTHER_CHARACTER(
	SerializePacket* pPacket,
	DWORD ID, BYTE Direction, short X, short Y, char HP) // headDirection
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_CREATE_OTHER_CHARACTER);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;
	*pPacket << Direction;
	*pPacket << X;
	*pPacket << Y;
	*pPacket << HP;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_DELETE_CHARACTER(
	SerializePacket* pPacket,
	DWORD ID)
{
	int size = sizeof(ID);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_DELETE_CHARACTER);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_MOVE_START(SerializePacket* pPacket,
	DWORD ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_MOVE_START);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;
	*pPacket << Direction;
	*pPacket << X;
	*pPacket << Y;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}


int SetSCPacket_MOVE_STOP(SerializePacket* pPacket,
	DWORD ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_MOVE_STOP);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;
	*pPacket << Direction;
	*pPacket << X;
	*pPacket << Y;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_ATTACK1(SerializePacket* pPacket,
	DWORD ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ATTACK1);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;
	*pPacket << Direction;
	*pPacket << X;
	*pPacket << Y;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_ATTACK2(SerializePacket* pPacket,
	DWORD ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ATTACK2);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;
	*pPacket << Direction;
	*pPacket << X;
	*pPacket << Y;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}


int SetSCPacket_ATTACK3(SerializePacket* pPacket,
	DWORD ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ATTACK3);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << ID;
	*pPacket << Direction;
	*pPacket << X;
	*pPacket << Y;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}



int SetSCPacket_DAMAGE(SerializePacket* pPacket,
	DWORD AttackID, DWORD DamageID, char DamageHP)
{
	int size = sizeof(AttackID) + sizeof(DamageID) + sizeof(DamageHP);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_DAMAGE);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << AttackID;
	*pPacket << DamageID;
	*pPacket << DamageHP;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}


int SetSCPacket_SYNC(SerializePacket* pPacket,
	DWORD ID, short X, short Y)
{
	int size = sizeof(ID) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_SYNC);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);


	*pPacket << ID;
	*pPacket << X;
	*pPacket << Y;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		return -1;
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_ECHO(SerializePacket* pPacket, int time)
{
	int size = sizeof(time);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ECHO);

	pPacket->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*pPacket << time;

	if (pPacket->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfPACKET_HEADER_SIZE + size);

		g_dump.Crash();
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return -1;
	}
	
	return (size + dfPACKET_HEADER_SIZE);
}
