#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> 
#include <cstring> 
#include <cstdlib>
#include <string> 
#include <Windows.h>
#include "Utils.h"

class Screen;
class Input;
class GameObject;
class Map;
class Block;

class Screen {
private:
	int	width;
	int	height;
	int	size;
	char* canvas;

	// constructor (생성자 함수) 메모리공간상에 적재되는 순간 호출되는
	Screen(int width = 14, int height = 22) // boundary |                |'\n' 12 + 2 + 1, 20 + 2
		: width(width), height(height), canvas(new char[(width + 1) * height])
	{
		bool faultyInput = false;
		if (this->width <= 0) {
			this->width = 15;
			faultyInput = true;
		}
		if (this->height <= 0) {
			this->height = 22;
			faultyInput = true;
		}
		size = (this->width + 1) * this->height;
		if (faultyInput == true) {
			delete canvas;
			canvas = new char[size];
		}
	}
	// destructor (소멸자 함수) 메모리공간상에서 없어지는 순간 호출되는 함수
	virtual ~Screen() {
		delete[] canvas;
		canvas = nullptr;
		width = 0; height = 0;
	}
	static Screen* Instance;
public:
	static Screen* GetInstance() {
		if (Instance == nullptr) {
			Instance = new Screen;
		}
		return Instance;
	}
	int getWidth() const {
		return width;
	}
	int getHeight() const {
		return height;
	}
	int getSize() const {
		return size;
	}
	void clear() {
		memset(canvas, ' ', size);
	}
	void render() {
		Borland::gotoxy(0, 0);
		for (int h = 0; h < height; h++)
			canvas[(width + 1) * (h + 1) - 1] = '\n';
		canvas[size - 1] = '\0';
		printf("%s", canvas);
	}
};
Screen* Screen::Instance = nullptr;

class Input {
	DWORD cNumRead, fdwMode;
	INPUT_RECORD irInBuf[128];
	HANDLE hStdin;
	DWORD fdwSaveOldMode;
	void errorExit(const char*);

	static Input* Instance;

	Input()	{
		hStdin = GetStdHandle(STD_INPUT_HANDLE);
		if (hStdin == INVALID_HANDLE_VALUE)
			errorExit("GetStdHandle");
		if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
			errorExit("GetConsoleMode");
		fdwMode = ENABLE_EXTENDED_FLAGS;
		if (!SetConsoleMode(hStdin, fdwMode))
			errorExit("SetConsoleMode");
		fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		if (!SetConsoleMode(hStdin, fdwMode))
			errorExit("SetConsoleMode");
	}
	~Input() {
		SetConsoleMode(hStdin, fdwSaveOldMode);
	}
public:
	static Input* GetInstance() {
		if (Instance == nullptr) {
			Instance = new Input;
		}
		return Instance;
	}
	void readInputs() {
		if (!GetNumberOfConsoleInputEvents(hStdin, &cNumRead)) {
			cNumRead = 0;
			return;
		}
		if (cNumRead == 0) return;

		if (!ReadConsoleInput(
			hStdin,
			irInBuf,
			128,
			&cNumRead))
			errorExit("ReadConsoleInput");
	}
	bool getKeyDown(WORD virtualKeyCode);
	bool getKey(WORD virtualKeyCode);
	bool getKeyUp(WORD virtualKeyCode);
};
Input* Input::Instance = nullptr;

class GameObject {
	Screen* screen;
	
public:
	GameObject()
		:screen(Screen::GetInstance())
	{

	}
	int getWidth() const {
		return screen->getWidth();
	}
	int getHeight() const {
		return screen->getHeight();
	}
	int getSize() const {
		return screen->getSize();
	}
};

class Map : public GameObject {
	char* board;

public:
	Map()
		: board(new char[getSize()])
	{
	}
	~Map() {
	}
	Position Index2Pos(int offset) const {

	}
	int pos2Index(const Position& pos) const {
		return (getWidth() + 1) * pos.y + pos.x; // x + 15y
	}
	void draw(const Position& pos, const char* shape, const Dimension& dm = Position{ 1, 1 }) {
		int index = pos2Index(pos);
		for (int h = 0; h < dm.y; h++)
			strncpy(&canvas[index + (width + 1) * h], &shape[h * dm.x], dm.x);
	}
	void eraseLines() {
		for (int h = 1; h < getHeight() - 1; h++) {
			bool isFull = true;
			isFull = checkLineFull(h);
			if (isFull) {
				moveBlockDown(h);
			}
		}
	}
	bool checkLineFull(int h) const {
		for (int i = 1; i < getWidth() - 1; i++) {
			Position pos{ i, h };
			if (board[pos2Index(pos)] != '*') {
				return false;
			}
		}
		return true;
	}
	void moveBlockDown(int h) {
		for (int i = h - 1; i > 0; i--) {
			for (int j = 1; j < getWidth() - 1; j++)
			{
				Position pos2{ j, i };
				board[pos2Index(pos2) + 15] = board[pos2Index(pos2)];
			}
		}
	}
	bool gameOver() {
	}
};

enum class Shape {
	I = 1,
	J = 2,
	L = 3,
	O = 4,
	S = 5,
	T = 6,
	Z = 7 
};

class Block {
	char shape[16];
	int shapeNum;
	Dimension dm;
	Position pos;
public:
	Block()
		: shape{ ' ' }, shapeNum(shapeNum), dm{ 1,1 }, pos{ 0,0 }
	{
		srand((unsigned)time(nullptr));
		shapeNum = rand() % 7 + 1;
	}
	void generateRandomBlock() {
		if (shapeNum == 1) {
			strcpy(shape, "            ****");
			dm = { 4,4 };
		}
		else if (shapeNum == 2) {
			strcpy(shape, "*  ***");
			dm = { 3,2 };
		}
		else if (shapeNum == 3) {
			strcpy(shape, "  ****");
			dm = { 3,2 };
		}
		else if (shapeNum == 4) {
			strcpy(shape, "****");
			dm = { 2,2 };
		}
		else if (shapeNum == 5) {
			strcpy(shape, " **** ");
			dm = { 3,2 };
		}
		else if (shapeNum == 6) {
			strcpy(shape, " * ***");
			dm = { 3,2 };
		}
		else if (shapeNum == 7) {
			strcpy(shape, "**  **");
			dm = { 3,2 };
		}
	}
	void turnBlock() {
		Dimension fourXfour{ 4,4 };
		Dimension threeXtwo{ 3,2 };
		Dimension twoXthree{ 2,3 };
		if (this->dm.x == fourXfour.x && this->dm.y == fourXfour.y) {
			char temp[16] = { ' ' };
			temp[0] = this->shape[3];
			temp[1] = this->shape[7];
			temp[2] = this->shape[11];
			temp[3] = this->shape[15];
			temp[4] = this->shape[2];
			temp[5] = this->shape[6];
			temp[6] = this->shape[10];
			temp[7] = this->shape[14];
			temp[8] = this->shape[1];
			temp[9] = this->shape[5];
			temp[10] = this->shape[9];
			temp[11] = this->shape[13];
			temp[12] = this->shape[0];
			temp[13] = this->shape[4];
			temp[14] = this->shape[8];
			temp[15] = this->shape[12];

			for (int i = 0; i < 15; i++)
			{
				this->shape[i] = temp[i];
			}
		}
		else if (this->dm.x == threeXtwo.x && this->dm.y == threeXtwo.y) {
			char temp[16] = { ' ' };
			temp[0] = shape[2];
			temp[1] = shape[5];
			temp[2] = shape[1];
			temp[3] = shape[4];
			temp[4] = shape[0];
			temp[5] = shape[3];

			for (int i = 0; i < 6; i++)
			{
				this->shape[i] = temp[i];
			}
		}
		else if (this->dm.x == twoXthree.x && this->dm.y == twoXthree.y) {
			char temp[16] = { ' ' };
			temp[0] = shape[1];
			temp[1] = shape[3];
			temp[2] = shape[5];
			temp[3] = shape[0];
			temp[4] = shape[2];
			temp[5] = shape[4];

			for (int i = 0; i < 6; i++)
			{
				this->shape[i] = temp[i];
			}
		}
	}
};

int main()
{

	return 0;
}

void Input::errorExit(const char* lpszMessage)
{
	fprintf(stderr, "%s\n", lpszMessage);
	SetConsoleMode(hStdin, fdwSaveOldMode);
	ExitProcess(0);
}
bool Input::getKeyDown(WORD virtualKeyCode) {
	return getKey(virtualKeyCode);
}
bool Input::getKey(WORD virtualKeyCode) {
	if (cNumRead == 0) return false; // 현재 콘솔에서 읽은 갯수가 0이냐?

	for (int i = 0; i < cNumRead; i++)
	{
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode && irInBuf[i].Event.KeyEvent.bKeyDown == TRUE) {
			return true;
		}
		return false;
	}
}
bool Input::getKeyUp(WORD virtualKeyCode) {
	if (cNumRead == 0) return false; // 현재 콘솔에서 읽은 갯수가 0이냐?

	for (int i = 0; i < cNumRead; i++)
	{
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode && irInBuf[i].Event.KeyEvent.bKeyDown == FALSE) {
			return true;
		}
		return false;
	}
}