#pragma once

// 수평, 수직 단위 비용이 10이면, 대각선은 sqrt(2) * 10 = 14.14라서 14로 지정
#define VERT_DIST 10
#define DIAG_DIST 14

class Pos
{
public:
	Pos() : _x(0), _y(0) {}
	Pos(int x, int y) : _x(x), _y(y) {}

public:
	int _x = 0;
	int _y = 0;

public:
	Pos& operator=(const Pos& other);
	Pos operator+(const Pos& other) { return Pos(_x + other._x, _y + other._y); }
	bool operator==(const Pos& other) { return (_x == other._x && _y == other._y); }
	bool operator!=(const Pos& other) { return (_x != other._x || _y != other._y); }
	bool operator < (const Pos& other) const;
	bool operator > (const Pos& other) const;

public:
	int GetDistance(Pos pos2, bool vert);	// Euclidean
	int GetDistanceToDest(Pos destPos);		// Manhattan
};
