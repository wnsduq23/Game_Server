#pragma once
#include "Protocol.h"
#include "SerializePacket.h"

void SetSCPacket_HEADER(
	stPACKET_HEADER& header,
	BYTE Size, BYTE Type);

int SetSCPacket_CREATE_MY_CHARACTER(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y, BYTE HP);

int SetSCPacket_CREATE_OTHER_CHARACTER(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y, BYTE HP);

int SetSCPacket_DELETE_CHARACTER(
	SerializePacket* buffer,
	int ID);

int SetSCPacket_MOVE_START(
	SerializePacket* buffe_r,
	int ID, BYTE Direction, short X, short Y);

int SetSCPacket_MOVE_STOP(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y);

int SetSCPacket_ATTACK1(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y);

int SetSCPacket_ATTACK2(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y);

int SetSCPacket_ATTACK3(
	SerializePacket* buffer,
	int ID, BYTE Direction, short X, short Y);

int SetSCPacket_DAMAGE(
	SerializePacket* buffer,
	int AttackID, int DamageID, BYTE DamageHP);

int SetSCPacket_SYNC(
	SerializePacket* buffer,
	int ID, short X, short Y);

int SetSCPacket_ECHO(
	SerializePacket* buffer,
	int time);
