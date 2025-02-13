#include "SetSCPacket.h"
#include <stdio.h>

void SetSCPacket_HEADER(
	stPACKET_HEADER& header,
	BYTE Size, BYTE Type)
{
	header.code = dfPACKET_HEADER_CODE;
	header.payload_size = Size;
	header.action_type = Type;
}

int SetSCPacket_CREATE_MY_CHARACTER(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y, BYTE HP) // headDirection
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_CREATE_MY_CHARACTER);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;
	*buffer << HP;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_CREATE_OTHER_CHARACTER(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y, BYTE HP) // headDirection
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_CREATE_OTHER_CHARACTER);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;
	*buffer << HP;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_DELETE_CHARACTER(
	SerializePacket* buffer,
	int ID)
{
	int size = sizeof(ID);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_DELETE_CHARACTER);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_MOVE_START(SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_MOVE_START);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}


int SetSCPacket_MOVE_STOP(SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_MOVE_STOP);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_ATTACK1(SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ATTACK1);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_ATTACK2(SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ATTACK2);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}


int SetSCPacket_ATTACK3(SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ATTACK3);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}



int SetSCPacket_DAMAGE(SerializePacket* buffer,
	int AttackID, int DamageID, BYTE DamageHP)
{
	int size = sizeof(AttackID) + sizeof(DamageID) + sizeof(DamageHP);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_DAMAGE);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << AttackID;
	*buffer << DamageID;
	*buffer << DamageHP;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}


int SetSCPacket_SYNC(SerializePacket* buffer,
	int ID, short X, short Y)
{
	int size = sizeof(ID) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_SYNC);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);


	*buffer << ID;
	*buffer << X;
	*buffer << Y;

	if (buffer->GetReadPtr() + size + dfPACKET_HEADER_SIZE != buffer->GetWritePtr())
	{
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}

	return (size + dfPACKET_HEADER_SIZE);
}

int SetSCPacket_ECHO(SerializePacket* buffer, int time)
{
	int size = sizeof(time);
	stPACKET_HEADER header;
	SetSCPacket_HEADER(header, size, dfPACKET_SC_ECHO);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << time;

	if (buffer->GetDataSize() != dfPACKET_HEADER_SIZE + size)
	{
		/*LOG(L"FightGame", CSystemLog::ERROR_LEVEL,
			L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfHEADER_SIZE + size);

		::wprintf(L"%s[%d]: Create Packet Error, %d != %llu\n",
			_T(__FUNCTION__), __LINE__, pPacket->GetDataSize(), dfHEADER_SIZE + size);

		dump.Crash();*/
		printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		return (-1);
	}
	
	return (size + dfPACKET_HEADER_SIZE);
}
