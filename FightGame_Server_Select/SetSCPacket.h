#pragma once
#include "Protocol.h"
#include "SerializePacket.h"

void SetSCPacket_HEADER(
	stPACKET_HEADER& header,
	BYTE Size, BYTE Type);

int SetSCPacket_CREATE_MY_CHARACTER(
	SerializePacket* buffer,
	DWORD ID, BYTE Direction, short X, short Y, char HP);

int SetSCPacket_CREATE_OTHER_CHARACTER(
	SerializePacket* buffer,
	DWORD ID, BYTE Direction, short X, short Y, char HP);

int SetSCPacket_DELETE_CHARACTER(
	SerializePacket* buffer,
	DWORD ID);

int SetSCPacket_MOVE_START(
	SerializePacket* buffe_r,
	DWORD ID, BYTE Direction, short X, short Y);

int SetSCPacket_MOVE_STOP(
	SerializePacket* buffer,
	DWORD ID, BYTE Direction, short X, short Y);

int SetSCPacket_ATTACK1(
	SerializePacket* buffer,
	DWORD ID, BYTE Direction, short X, short Y);

int SetSCPacket_ATTACK2(
	SerializePacket* buffer,
	DWORD ID, BYTE Direction, short X, short Y);

int SetSCPacket_ATTACK3(
	SerializePacket* buffer,
	DWORD ID, BYTE Direction, short X, short Y);

int SetSCPacket_DAMAGE(
	SerializePacket* buffer,
	DWORD AttackID, DWORD DamageID, char DamageHP);

int SetSCPacket_SYNC(
	SerializePacket* buffer,
	DWORD ID, short X, short Y);

int SetSCPacket_ECHO(
	SerializePacket* buffer,
	int time);
