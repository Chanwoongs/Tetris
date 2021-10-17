#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> 
#include <cstring> 
#include <cstdlib>
#include <string> 
#include <Windows.h>
#include <vector>
#include "Utils.h"

using namespace std;

class Screen;
class Input;
class GameObject;
class Block;
class Map;
class GameManager;

//map
class Screen {
private:
	int	width;
	int	height;
	int	size;
	char* canvas;

	Screen(int width = 14, int height = 22) // boundary |                |'\n' 12 + 2 + 1, 20 + 2
		: width(width), height(height), canvas(new char[(width + 1) * height])
	{
		bool faultyInput = false;
		if (this->width <= 0) {
			this->width = 14;
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

	// Position을 Index로 변환
	int pos2Index(const Position& pos) const
	{
		return (getWidth() + 1) * pos.y + pos.x; // x + 15y
	}

	void draw(Map* map);

	// 화면 출력
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

	Input() {
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

	// Input 받기
	void readInputs() {
		if (!GetNumberOfConsoleInputEvents(hStdin, &cNumRead))
		{
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
protected:
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

	int pos2Index(Position pos) {
		return screen->pos2Index(pos);
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
class Block : public GameObject {
	char* shape;
	int shapeNum;
	bool isMoving;
	Dimension dm;
	Position pos;

public:
	Block() /// pos 수정 screen 값/ 2로
		: shape(shape), shapeNum(shapeNum), dm(4, 4), pos{ getWidth() / 2 , 0 }, isMoving(true)
	{
		srand((unsigned)time(nullptr));

		// shape 랜덤 선정
		shapeNum = rand() % 7 + 1;

		// shape과 dm 초기화
		// I Block
		if (shapeNum == (int)Shape::I) {
			shape = new char[5];
			strncpy(shape, "****", sizeof("****"));
			dm = { 1,4 };
		}
		// J Block
		else if (shapeNum == (int)Shape::J) {
			shape = new char[7];
			strncpy(shape, "*  ***", sizeof("*  ***"));
			dm = { 3,2 };
		}
		// L Block
		else if (shapeNum == (int)Shape::L) {
			shape = new char[7];
			strncpy(shape, "* * **", sizeof("* * **"));
			dm = { 2,3 };
		}
		// O Block
		else if (shapeNum == (int)Shape::O) {
			shape = new char[5];
			strncpy(this->shape, "****", sizeof("****"));
			dm = { 2,2 };
		}
		// S Block
		else if (shapeNum == (int)Shape::S) {
			shape = new char[7];
			strncpy(shape, " **** ", sizeof(" **** "));
			dm = { 3,2 };
		}
		// T Block
		else if (shapeNum == (int)Shape::T) {
			shape = new char[7];
			strncpy(shape, "*** * ", sizeof("*** * "));
			dm = { 3,2 };
		}
		// Z Block
		else if (shapeNum == (int)Shape::Z) {
			shape = new char[7];
			strncpy(shape, "**  **", sizeof("**  **"));
			dm = { 3,2 };
		}
	}
	~Block() {
		delete[] shape;
	}

	Position getPos() {
		return pos;
	}

	void setPos(int x, int y) {
		pos.x = x;
		pos.y = y;
	}

	int getShapeNum() const {
		return shapeNum;
	}

	int getBlockIndex() {
		return pos2Index(pos);
	}

	Dimension getDimension() const {
		return dm;
	}

	void setDimension(int x, int y) {
		dm.x = x;
		dm.y = y;
	}

	const char* getShape() const {
		return shape;
	}

	void setShape(int i, char c) {
		shape[i] = c;
	}

	void setMovingFlag(bool isMoving) {
		this->isMoving = isMoving;
	}

	bool getIsMoving() const {
		return isMoving;
	}
};

//screen
class Map : public GameObject {
	char* board;

public:
	Map()
		: board(new char[getSize()])
	{
	}
	~Map() {
		delete[] board;
	}

	// 보드 초기화
	void initializeBoard()
	{
		memset(board, ' ', getSize());

		for (int i = 15; i < 311; i += 15)
		{
			board[i] = '@';
		}
		for (int i = 28; i < 314; i += 15)
		{
			board[i] = '@';
		}
		for (int i = 0; i < 14; i++)
		{
			board[i] = '@';
		}
		for (int i = 315; i < 329; i++)
		{
			board[i] = '@';
		}
		for (int i = 14; i < 315; i += 15)
		{
			board[i] = '\n';
		}
		board[getSize() - 1] = '\0';
	}

	Position Index2Pos(int offset) const {

	}

	char* getBoard() {
		return board;
	}

	// ActiveBlock 그리기
	void drawBlock(Block& block) {
		if (block.getIsMoving())
		{
			// 빈칸 구분 없이 \0 전까지만 그린다
			if ((block.getShapeNum() == 1 && block.getDimension().comparePos(1, 4)) || (block.getShapeNum() == 1 && block.getDimension().comparePos(4, 1)) ||
				(block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && block.getShape()[0] == ' ') ||
				(block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && block.getShape()[0] != ' ') ||
				(block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && block.getShape()[0] != ' ') ||
				(block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && block.getShape()[0] == ' ') ||
				(block.getShapeNum() == 4) ||
				(block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && block.getShape()[0] == ' '))
			{
				for (int h = 0; h < block.getDimension().y; h++)
				{
					strncpy(&board[pos2Index(block.getPos()) + (getWidth() + 1) * h], &block.getShape()[h * block.getDimension().x], block.getDimension().x);
				}
			}

			// 빈칸은 그리지 않고 \0 전까지만 그린다
			if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2))
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 2] = block.getShape()[2];
				board[block.getBlockIndex() + 17] = block.getShape()[5];
			}
			if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3))
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 15] = block.getShape()[2];
				board[block.getBlockIndex() + 30] = block.getShape()[4];
			}
			if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2))
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 2] = block.getShape()[2];
				board[block.getBlockIndex() + 15] = block.getShape()[3];
			}
			if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3))
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 16] = block.getShape()[3];
				board[block.getBlockIndex() + 31] = block.getShape()[5];
			}
			if (block.getShapeNum() == 5 && block.getDimension().comparePos(3, 2))
			{
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 2] = block.getShape()[2];
				board[block.getBlockIndex() + 15] = block.getShape()[3];
				board[block.getBlockIndex() + 16] = block.getShape()[4];
			}
			if (block.getShapeNum() == 5 && block.getDimension().comparePos(2, 3))
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 15] = block.getShape()[2];
				board[block.getBlockIndex() + 16] = block.getShape()[3];
				board[block.getBlockIndex() + 31] = block.getShape()[5];
			}
			if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2))
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 2] = block.getShape()[2];
				board[block.getBlockIndex() + 16] = block.getShape()[4];
			}
			if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && block.getShape()[0] == ' ')
			{
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 15] = block.getShape()[2];
				board[block.getBlockIndex() + 16] = block.getShape()[3];
				board[block.getBlockIndex() + 31] = block.getShape()[5];
			}
			if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && block.getShape()[0] != ' ')
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 15] = block.getShape()[2];
				board[block.getBlockIndex() + 16] = block.getShape()[3];
				board[block.getBlockIndex() + 30] = block.getShape()[4];
			}
			if (block.getShapeNum() == 7 && block.getDimension().comparePos(3, 2))
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 16] = block.getShape()[4];
				board[block.getBlockIndex() + 17] = block.getShape()[5];
			}
			if (block.getShapeNum() == 7 && block.getDimension().comparePos(2, 3))
			{
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 15] = block.getShape()[2];
				board[block.getBlockIndex() + 16] = block.getShape()[3];
				board[block.getBlockIndex() + 30] = block.getShape()[4];
			}
		}
	}

	// board에 남아있는 Block 그리기
	void drawFixedBlocks(vector<Block*>& fixedBlocks) {
		//매니저에서 고정 블럭들의 shape 받아서 draw
		vector<Block*>::iterator it;
		for (it = fixedBlocks.begin(); it != fixedBlocks.end() && !fixedBlocks.empty(); it++)
		{
			// 비어있는 블록을 fixedBlocks 에서 지운다 
			// Shape I, O // *it == Block*
			if (((*it)->getDimension().comparePos(1, 4) || (*it)->getDimension().comparePos(4, 1) || (*it)->getDimension().comparePos(2, 2)) && ((*it)->getShape() == "    "))
			{
				fixedBlocks.erase(it);
				continue;
			}

			// Shape J, L, S, T, Z
			else if (((*it)->getDimension().comparePos(2, 3) || (*it)->getDimension().comparePos(3, 21)) && (*it)->getShape() == "      ")
			{
				fixedBlocks.erase(it);
				continue;
			}

			// 남아있는 블럭 그리기
			if (((*it)->getShapeNum() == 1 && (*it)->getDimension().comparePos(1, 4)) || ((*it)->getShapeNum() == 1 && (*it)->getDimension().comparePos(4, 1)) ||
				((*it)->getShapeNum() == 2 && (*it)->getDimension().comparePos(2, 3) && (*it)->getShape()[0] == ' ') ||
				((*it)->getShapeNum() == 2 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[0] != ' ') ||
				((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(2, 3) && (*it)->getShape()[0] != ' ') ||
				((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[0] == ' ') ||
				((*it)->getShapeNum() == 4) ||
				((*it)->getShapeNum() == 6 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[0] == ' '))
			{
				for (int h = 0; h < (*it)->getDimension().y; h++)
				{
					strncpy(&board[pos2Index((*it)->getPos()) + (getWidth() + 1) * h], &((*it)->getShape()[h * (*it)->getDimension().x]), (*it)->getDimension().x);
				}
			}
			if ((*it)->getShapeNum() == 2 && (*it)->getDimension().comparePos(3, 2))
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 2] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 17] = (*it)->getShape()[5];
			}
			if ((*it)->getShapeNum() == 2 && (*it)->getDimension().comparePos(2, 3))
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 30] = (*it)->getShape()[4];
			}
			if ((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(3, 2))
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 2] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 17] = (*it)->getShape()[5];
			}
			if ((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(2, 3))
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[3];
				board[(*it)->getBlockIndex() + 31] = (*it)->getShape()[5];
			}
			if ((*it)->getShapeNum() == 5 && (*it)->getDimension().comparePos(3, 2))
			{
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 2] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[3];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[4];
			}
			if ((*it)->getShapeNum() == 5 && (*it)->getDimension().comparePos(2, 3))
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[3];
				board[(*it)->getBlockIndex() + 31] = (*it)->getShape()[5];
			}
			if ((*it)->getShapeNum() == 6 && (*it)->getDimension().comparePos(3, 2))
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 2] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[4];
			}
			if ((*it)->getShapeNum() == 6 && (*it)->getDimension().comparePos(2, 3) && (*it)->getShape()[0] == ' ')
			{
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[3];
				board[(*it)->getBlockIndex() + 31] = (*it)->getShape()[5];
			}
			if ((*it)->getShapeNum() == 6 && (*it)->getDimension().comparePos(2, 3) && (*it)->getShape()[0] != ' ')
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[3];
				board[(*it)->getBlockIndex() + 30] = (*it)->getShape()[4];
			}
			if ((*it)->getShapeNum() == 7 && (*it)->getDimension().comparePos(3, 2))
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[4];
				board[(*it)->getBlockIndex() + 17] = (*it)->getShape()[5];
			}
			if ((*it)->getShapeNum() == 7 && (*it)->getDimension().comparePos(2, 3))
			{
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 16] = (*it)->getShape()[3];
				board[(*it)->getBlockIndex() + 30] = (*it)->getShape()[4];
			}
		}
	}

	// 줄 지우기
	void eraseLines()
	{
		for (int h = 1; h < getHeight() - 1; h++)
		{
			bool isFull = false;
			// 줄이 다 찼는지 확인
			isFull = checkLinesFull(h);

			if (isFull)
			{
				// 윗 라인 모두 아래로 내리기
				moveBlocksDown(h);
			}
		}
	}

	// 줄이 다 찼는지 확인
	bool checkLinesFull(int h) const
	{
		for (int i = 1; i < getWidth() - 1; i++)
		{
			Position pos{ i, h };

			if (board[screen->pos2Index(pos)] != '*')
			{
				return false;
			}
		}
		return true;
	}

	// 윗 라인 모두 아래로 내리기
	void moveBlocksDown(int h)
	{
		for (int i = h - 1; i > 0; i--) {
			for (int j = 1; j < getWidth() - 1; j++)
			{
				Position pos2{ j, i };
				board[pos2Index(pos2) + 15] = board[pos2Index(pos2)];
			}
		}
	}

	// 블럭 멈추기
	void freezeBlock(Block& block) {
		// shape ㅣ
		if (block.getShapeNum() == 1 && block.getDimension().comparePos(1, 4)) {
			if (board[pos2Index(block.getPos().addPos(0, 4))] != ' ') {
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 1 && block.getDimension().comparePos(4, 1)) {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ' || board[pos2Index(block.getPos().addPos(3, 1))] != ' ') {
				block.setMovingFlag(false);
			}
		}
		// shape J
		if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos().addPos(0, 1))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos().addPos(0, 1))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape L
		if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape O
		if (block.getShapeNum() == 4) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape S
		if (block.getShapeNum() == 5 && block.getDimension().comparePos(2, 3)) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 5 && block.getDimension().comparePos(3, 2)) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape T
		if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape Z
		if (block.getShapeNum() == 7 && block.getDimension().comparePos(2, 3)) {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		if (block.getShapeNum() == 7 && block.getDimension().comparePos(3, 2)) {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
	}

	// 블럭 공백 채우기
	void fillBlank(Block& block) {
		for (int i = 0; i < strlen(block.getShape()); i++)
		{
			if (block.getShape()[i] == ' ' && board[pos2Index(block.getPos()) + 15] == '*')
			{
				block.setShape(i, '*');
			}
		}
	}

	void update(Block& block, vector<Block*>& fixedBlocks)
	{
		initializeBoard();
		drawFixedBlocks(fixedBlocks);
		drawBlock(block); // activeBlock draw
		freezeBlock(block);
		eraseLines();
	}
};


class GameManager {
	Map* map;
	Screen* screen;
	Input* input;
	Block* activeBlock;
	Block* nextBlock;
	vector<Block*> fixedBlocks;

	bool isLooping;
	bool isFall;

public:
	GameManager()
		: map(new Map), screen(Screen::GetInstance()), input(Input::GetInstance()), activeBlock(new Block), nextBlock(new Block), isFall(false), isLooping(true)
	{
	}
	~GameManager()
	{
		delete nextBlock;
		delete activeBlock;
		delete map;
	}

	// 게임 시작
	void gameStart() {
		map->initializeBoard();

		while (isLooping) {
			screen->clear();
			screen->draw(map);
			input->readInputs();
			update();
			screen->render();
			Sleep(150);
		}
	}

	void update() {
		if (isGameOver(fixedBlocks)) {
			isLooping = false;
			Borland::gotoxy(screen->getWidth() + 5, screen->getHeight() - 10);
			printf("GAME OVER");
		}
		map->update(*activeBlock, fixedBlocks);
		createNewBlock();
		moveBlock();
	}

	// 새로운 블럭 생성 함수
	void createNewBlock() {
		if (activeBlock->getIsMoving()) return;

		// 떨어진 블럭 정보 저장
		fixedBlocks.push_back(activeBlock);

		// 다음 블록 미리보기
		activeBlock = nextBlock;
		nextBlock = new Block;
	}

	// 블럭 움직임
	void moveBlock() {
		if (!activeBlock->getIsMoving()) return;

		// 블럭 한칸씩 밑으로 움직임
		activeBlock->setPos(activeBlock->getPos().x, activeBlock->getPos().y + 1);

		// 키입력
		if (input->getKeyUp(VK_UP))
		{
			turnBlock(*activeBlock);
		}
		if (input->getKey(VK_LEFT))
		{
			if (checkLeftCollision()) return;
			activeBlock->setPos(activeBlock->getPos().x - 1, activeBlock->getPos().y);
		}
		if (input->getKey(VK_RIGHT))
		{
			if (checkRightCollision()) return;
			activeBlock->setPos(activeBlock->getPos().x + 1, activeBlock->getPos().y);
		}
	}

	// 블럭 반시계 방향으로 돌리기
	void turnBlock(Block& block) {
		// 1 X 4 -> 4 X 1
		if (block.getDimension().comparePos(1, 4))
		{
			if (map->getBoard()[screen->pos2Index(block.getPos().addPos(3, 0))] != ' ') return;

			char temp[4] = { ' ' };
			temp[0] = block.getShape()[0];
			temp[1] = block.getShape()[1];
			temp[2] = block.getShape()[2];
			temp[3] = block.getShape()[3];

			for (int i = 0; i < 4; i++)
			{
				block.setShape(i, temp[i]);
			}
			block.setDimension(4, 1);

		}
		// 4 X 1 -> 1 X 4
		else if (block.getDimension().comparePos(4, 1))
		{
			if (map->getBoard()[screen->pos2Index(block.getPos().addPos(0, 3))] != ' ') return;

			char temp[4] = { ' ' };
			temp[0] = block.getShape()[3];
			temp[1] = block.getShape()[2];
			temp[2] = block.getShape()[1];
			temp[3] = block.getShape()[0];

			for (int i = 0; i < 4; i++)
			{
				block.setShape(i, temp[i]);
			}
			block.setDimension(1, 4);
		}
		// 3 X 2 -> 2 X 3
		else if (block.getDimension().comparePos(3, 2))
		{
			char temp[6] = { ' ' };
			temp[0] = block.getShape()[2];
			temp[1] = block.getShape()[5];
			temp[2] = block.getShape()[1];
			temp[3] = block.getShape()[4];
			temp[4] = block.getShape()[0];
			temp[5] = block.getShape()[3];

			for (int i = 0; i < 6; i++)
			{
				block.setShape(i, temp[i]);
			}
			block.setDimension(2, 3);
		}
		// 2 X 3 -> 3 X 2
		else if (block.getDimension().comparePos(2, 3))
		{
			char temp[6] = { ' ' };
			temp[0] = block.getShape()[1];
			temp[1] = block.getShape()[3];
			temp[2] = block.getShape()[5];
			temp[3] = block.getShape()[0];
			temp[4] = block.getShape()[2];
			temp[5] = block.getShape()[4];

			for (int i = 0; i < 6; i++)
			{
				block.setShape(i, temp[i]);
			}
			block.setDimension(3, 2);
		}
	}


	// 왼쪽 벽 충돌 처리
	bool checkLeftCollision()
	{
		// Dimension (4, 1) 
		if (activeBlock->getDimension().comparePos(1, 4)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos()) - 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 1)) - 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 2)) - 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 3)) - 1] != ' ') return true;
		}

		// Dimension (1, 4)
		if (activeBlock->getDimension().comparePos(4, 1)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos()) - 1] != ' ') return true;
		}

		// Dimension (2, 2)
		if (activeBlock->getDimension().comparePos(2, 2)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos()) - 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 1)) - 1] != ' ') return true;
		}

		// Dimension (3, 2)
		if (activeBlock->getDimension().comparePos(3, 2)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos()) - 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 1)) - 1] != ' ') return true;
		}

		// Dimension (2, 3)
		if (activeBlock->getDimension().comparePos(2, 3)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos()) - 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 1)) - 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 2)) - 1] != ' ') return true;
		}

		else return false;
	}

	// 오른쪽 벽 충돌 처리
	bool checkRightCollision()
	{
		// Dimension (4, 1) 
		if (activeBlock->getDimension().comparePos(1, 4)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos()) + 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 1)) + 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 2)) + 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(0, 3)) + 1] != ' ') return true;
		}

		// Dimension (1, 4)
		if (activeBlock->getDimension().comparePos(4, 1)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(3, 0)) + 1] != ' ') return true;
		}

		// Dimension (2, 2)
		if (activeBlock->getDimension().comparePos(2, 2)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(1, 0)) + 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(1, 1)) + 1] != ' ') return true;
		}

		// Dimension (3, 2)
		if (activeBlock->getDimension().comparePos(3, 2)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(2, 0)) + 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(2, 1)) + 1] != ' ') return true;
		}

		// Dimension (2, 3)
		if (activeBlock->getDimension().comparePos(2, 3)) {
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(1, 0)) + 1] != ' ') return true;
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(1, 1)) + 1] != ' ') return true;
			if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(1, 2)) + 1] != ' ') return true;
		}

		else return false;
	}

	// 게임 오버 조건 확인
	bool isGameOver(vector<Block*>& fixedBlocks)
	{
		vector<Block*>::iterator it;
		for (it = fixedBlocks.begin(); it != fixedBlocks.end() && !fixedBlocks.empty(); it++)
		{
			if ((*it)->getPos().y - 1 <= 0) {
				return true;
			}
		}
		return false;
	}
};


// 보드 스크린으로 가져오기
void Screen::draw(Map* map) {
	char* temp = map->getBoard();
	for (int i = 0; i < size; i++) {
		canvas[i] = temp[i];
	}
}

int main()
{
	GameManager gm;
	gm.gameStart();
	return 0;
}

void Input::errorExit(const char* lpszMessage)
{
	fprintf(stderr, "%s\n", lpszMessage);
	SetConsoleMode(hStdin, fdwSaveOldMode);
	ExitProcess(0);
}
bool Input::getKeyDown(WORD virtualKeyCode) {
	return false;
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