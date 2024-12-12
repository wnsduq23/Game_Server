#pragma once
#include "PacketDefine.h"
#include "SerializeBuffer.h"

void Create_PACKET_HEADER(
	stPACKET_HEADER& header,
	UINT8 Size, UINT8 Type);

int Create_PACKET_SC_CREATE_MY_CHARACTER(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y, UINT8 HP);

int Create_PACKET_SC_CREATE_OTHER_CHARACTER(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y, UINT8 HP);

int Create_PACKET_SC_DELETE_CHARACTER(
	SerializeBuffer* buffer,
	UINT32 ID);

int Create_PACKET_SC_MOVE_START(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y);

int Create_PACKET_SC_MOVE_STOP(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y);

int Create_PACKET_SC_ATTACK1(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y);

int Create_PACKET_SC_ATTACK2(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y);

int Create_PACKET_SC_ATTACK3(
	SerializeBuffer* buffer,
	UINT32 ID, UINT8 Direction, UINT16 X, UINT16 Y);

int Create_PACKET_SC_DAMAGE(
	SerializeBuffer* buffer,
	UINT32 AttackID, UINT32 DamageID, UINT8 DamageHP);

int Create_PACKET_SC_SYNC(
	SerializeBuffer* buffer,
	UINT16 X, UINT16 Y);
