#include "CreateSCPacket.h"

void Create_PACKET_HEADER(
	stPACKET_HEADER& header,
	UINT8 Size, UINT8 Type)
{
	header.code = dfPACKET_HEADER_CODE;
	header.payload_size = Size;
	header.action_type = Type;
}

int Create_PACKET_SC_CREATE_MY_CHARACTER(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y, UINT8 HP)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_CREATE_MY_CHARACTER);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;
	*buffer << HP;

	return (size + dfPACKET_HEADER_SIZE);
}

int Create_PACKET_SC_CREATE_OTHER_CHARACTER(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y, UINT8 HP)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_CREATE_OTHER_CHARACTER);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;
	*buffer << HP;

	return (size + dfPACKET_HEADER_SIZE);
}

int Create_PACKET_SC_DELETE_CHARACTER(
	SerializeBuffer* buffer,
	UINT32 ID)
{
	int size = sizeof(ID);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_DELETE_CHARACTER);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;

	return (size + dfPACKET_HEADER_SIZE);
}

int Create_PACKET_SC_MOVE_START(SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_MOVE_START);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	return (size + dfPACKET_HEADER_SIZE);
}


int Create_PACKET_SC_MOVE_STOP(SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_MOVE_STOP);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	return (size + dfPACKET_HEADER_SIZE);
}

int Create_PACKET_SC_ATTACK1(SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_ATTACK1);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	return (size + dfPACKET_HEADER_SIZE);
}

int Create_PACKET_SC_ATTACK2(SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_ATTACK2);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	return (size + dfPACKET_HEADER_SIZE);
}


int Create_PACKET_SC_ATTACK3(SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y)
{
	int size = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_ATTACK3);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << ID;
	*buffer << Direction;
	*buffer << X;
	*buffer << Y;

	return (size + dfPACKET_HEADER_SIZE);
}



int Create_PACKET_SC_DAMAGE(SerializeBuffer* buffer,
	UINT32 AttackID, UINT32 DamageID, UINT8 DamageHP)
{
	int size = sizeof(AttackID) + sizeof(DamageID) + sizeof(DamageHP);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_DAMAGE);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << AttackID;
	*buffer << DamageID;
	*buffer << DamageHP;

	return (size + dfPACKET_HEADER_SIZE);
}


int Create_PACKET_SC_SYNC(SerializeBuffer* buffer,
	UINT16 X, UINT16 Y)
{
	int size = sizeof(X) + sizeof(Y);
	stPACKET_HEADER header;
	Create_PACKET_HEADER(header, size, dfPACKET_SC_SYNC);

	buffer->PutData((char*)&header, dfPACKET_HEADER_SIZE);

	*buffer << X;
	*buffer << Y;

	return (size + dfPACKET_HEADER_SIZE);
}