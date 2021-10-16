#ifndef UTILS_H_
#define UTILS_H_

#include <Windows.h>

struct Position {
	int x;
	int y;
	Position(int x = 0, int y = 0) : x(x), y(y) {}

public:
	bool comparePos(Position pos) {
		if (this->x == pos.x && this->y == pos.y) {
			return true;
		}
	}
	bool comparePos(int x, int y) {
		if (this->x == x && this->y == y) {
			return true;
		}
		else return false;
	}
	Position addPos(Position pos) {
		Position newPos;
		int newX = 0;
		int newY = 0;
		newX = this->x + pos.x;
		newY = this->y + pos.y;

		newPos = (newX, newY);

		return newPos;
	}
	Position addPos(int x, int y) {
		Position newPos;
		newPos.x = this->x + x;
		newPos.y = this->y + y;

		return newPos;
	}
	Position operator+(Position& other) {
		other.x += this->x;
		other.y += this->y;

		return other;
	}
	void setPos(Position pos) {
		this->x = pos.x;
		this->y = pos.y;
	}
};

typedef Position Dimension;

class Borland {

public:
	static void initialize()
	{
		CONSOLE_CURSOR_INFO cci;
		cci.dwSize = 25;
		cci.bVisible = FALSE;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
	}

	static int wherex()
	{
		CONSOLE_SCREEN_BUFFER_INFO  csbiInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);
		return csbiInfo.dwCursorPosition.X;
	}
	static int wherey()
	{
		CONSOLE_SCREEN_BUFFER_INFO  csbiInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);
		return csbiInfo.dwCursorPosition.Y;
	}
	static void gotoxy(int x, int y)
	{
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), _COORD{ (SHORT)x, (SHORT)y });
	}
	static void gotoxy(const Position* pos)
	{
		if (!pos) return;
		gotoxy( (*pos).x, (*pos).y);
	}
	static void gotoxy(const Position& pos)
	{
		gotoxy( pos.x, pos.y);
	}
};

#endif 
