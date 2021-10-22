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
class UI;
class ScoreUI;
class PreviewUI;
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
		: shape(shape), shapeNum(shapeNum), dm(4, 4), pos{ getWidth() / 2 , 1 }, isMoving(true)
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
			strncpy(shape, "  ****", sizeof("  ****"));
			dm = { 3,2 };
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
				(block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && block.getShape()[1] == ' ') ||
				(block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && block.getShape()[1] == ' ') ||
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
			if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && block.getShape()[1] != ' ')
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 2] = block.getShape()[2];
				board[block.getBlockIndex() + 17] = block.getShape()[5];
			}
			if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && block.getShape()[0] != ' ')
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 15] = block.getShape()[2];
				board[block.getBlockIndex() + 30] = block.getShape()[4];
			}
			if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && block.getShape()[0] != ' ')
			{
				board[block.getBlockIndex()] = block.getShape()[0];
				board[block.getBlockIndex() + 1] = block.getShape()[1];
				board[block.getBlockIndex() + 2] = block.getShape()[2];
				board[block.getBlockIndex() + 15] = block.getShape()[3];
			}
			if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && block.getShape()[1] != ' ')
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
			if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && block.getShape()[0] != ' ')
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
				((*it)->getShapeNum() == 2 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[1] == ' ') ||
				((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(2, 3) && (*it)->getShape()[1] == ' ') ||
				((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[0] == ' ') ||
				((*it)->getShapeNum() == 4) ||
				((*it)->getShapeNum() == 6 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[0] == ' '))
			{
				for (int h = 0; h < (*it)->getDimension().y; h++)
				{
					strncpy(&board[pos2Index((*it)->getPos()) + (getWidth() + 1) * h], &((*it)->getShape()[h * (*it)->getDimension().x]), (*it)->getDimension().x);
				}
			}
			if ((*it)->getShapeNum() == 2 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[1] != ' ')
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 2] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 17] = (*it)->getShape()[5];
			}
			if ((*it)->getShapeNum() == 2 && (*it)->getDimension().comparePos(2, 3) && (*it)->getShape()[0] != ' ')
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 30] = (*it)->getShape()[4];
			}
			if ((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[0] != ' ')
			{
				board[(*it)->getBlockIndex()] = (*it)->getShape()[0];
				board[(*it)->getBlockIndex() + 1] = (*it)->getShape()[1];
				board[(*it)->getBlockIndex() + 2] = (*it)->getShape()[2];
				board[(*it)->getBlockIndex() + 15] = (*it)->getShape()[3];
			}
			if ((*it)->getShapeNum() == 3 && (*it)->getDimension().comparePos(2, 3) && (*it)->getShape()[1] != ' ')
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
			if ((*it)->getShapeNum() == 6 && (*it)->getDimension().comparePos(3, 2) && (*it)->getShape()[0] != ' ')
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
	void eraseLines(int& score, int& lines)
	{
		for (int h = 1; h < getHeight() - 1; h++)
		{
			bool isFull = false;
			// 줄이 다 찼는지 확인
			isFull = checkLinesFull(h);

			if (isFull)
			{
				// 윗 라인 모두 아래로 내리기
				moveBlocksDown(h, score, lines);
				isFull = false;
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
			else continue;
		}
		return true;
	}

	// 윗 라인 모두 아래로 내리기
	void moveBlocksDown(int h, int& score, int& lines)
	{
		for (int i = h - 1; i > 0; i--) {
			for (int j = 1; j < getWidth() - 1; j++)
			{
				Position pos2{ j, i };
				board[pos2Index(pos2) + 15] = board[pos2Index(pos2)];
			}
		}
		score += 100;
		lines += 1;
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

	void update(Block& block, vector<Block*>& fixedBlocks, int& score, int& lines)
	{
		initializeBoard();
		drawFixedBlocks(fixedBlocks);
		drawBlock(block); // activeBlock draw
		freezeBlock(block);
		eraseLines(score, lines);
	}
};

class UI : public GameObject {
protected:
	Position pos;
	char* uIBoard;
public:
	UI()
		: pos({ 0, 0 }), uIBoard(nullptr)
	{
	}
	virtual ~UI()
	{
	}
	virtual void drawUI() {}
	virtual void printUI(Position pos) {}
	virtual void printUI(int x, int y) {}
};

class ScoreUI : public UI {
	char scoreText[10];
	char linesText[10];
	char speedText[10];

public:
	ScoreUI()
		: scoreText{ " " }, linesText{ " " }, speedText{ " " }
	{
		this->pos.x = screen->getWidth() + 2;
		this->pos.y = screen->getHeight() - 21;

		this->uIBoard = new char[105];
	}
	~ScoreUI()
	{
		delete[] uIBoard;
	}

	// 정수 문자열 변환
	void int2String(int num, char text[])
	{
		sprintf(text, "%d", num);
	}

	// ScoreUI 화면에 그리기
	void drawUI(int& score, int& lines, int& speed)
	{
		// 초기화
		memset(uIBoard, ' ', 105);
		for (int i = 0; i < 20; i++)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 21; i < 85; i += 21)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 85; i < 104; i++)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 40; i < 104; i += 21)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 20; i < 84; i += 21)
		{
			uIBoard[i] = '\n';
		}
		uIBoard[104] = '\0';

		int2String(score, scoreText);
		int2String(lines, linesText);
		int2String(speed, speedText);

		// Score 
		strncpy(&uIBoard[22], "SCORE: ", sizeof("SCORE: "));
		strncpy(&uIBoard[29], scoreText, sizeof(scoreText));
		// Lines 
		strncpy(&uIBoard[43], "LINES: ", sizeof("LINES: "));
		strncpy(&uIBoard[50], linesText, sizeof(linesText));
		// Speed 
		strncpy(&uIBoard[64], "SPEED: ", sizeof("SPEED: "));
		strncpy(&uIBoard[71], speedText, sizeof(speedText));
	}

	void printUI(Position pos)
	{
		Borland::gotoxy(pos);
		for (int i = 0; i < 21; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(pos.addPos(0, 1));
		for (int i = 21; i < 42; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(pos.addPos(0, 2));
		for (int i = 42; i < 63; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(pos.addPos(0, 3));
		for (int i = 63; i < 84; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(pos.addPos(0, 4));
		for (int i = 84; i < 105; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(0, 0);
	}
};

class PreviewUI : public UI {

public:
	PreviewUI() {
		this->pos.x = screen->getWidth() + 2;
		this->pos.y = screen->getHeight() - 10;

		this->uIBoard = new char[63];
	}
	~PreviewUI() {
	}

	void drawUI(Block& nextBlock)
	{
		// 초기화
		memset(uIBoard, ' ', 63);
		for (int i = 0; i < 8; i++)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 9; i < 55; i += 9)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 55; i < 62; i++)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 16; i < 53; i += 9)
		{
			uIBoard[i] = ' ';
		}
		for (int i = 8; i < 62; i += 9)
		{
			uIBoard[i] = '\n';
		}
		uIBoard[62] = '\0';

		for (int h = 0; h < nextBlock.getDimension().y; h++)
		{
			strncpy(&uIBoard[21 + 9 * h], &nextBlock.getShape()[h * nextBlock.getDimension().x], nextBlock.getDimension().x);
		}
	}

	void printUI(int x, int y) 
	{
		Borland::gotoxy(x, y);
		for (int i = 0; i < 9; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(x, y + 1);
		for (int i = 9; i < 18; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(x, y + 2);
		for (int i = 18; i < 27; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(x, y + 3);
		for (int i = 27; i < 36; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(x, y + 4);
		for (int i = 36; i < 45; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(x, y + 5);
		for (int i = 45; i < 54; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(x, y + 6);
		for (int i = 54; i < 63; i++)
		{
			printf("%c", uIBoard[i]);
		}
		Borland::gotoxy(0, 0);
	}
};


class GameManager {
	Map* map;
	Screen* screen;
	Input* input;
	Block* activeBlock;
	Block* nextBlock;
	vector<Block*> fixedBlocks;
	ScoreUI scoreUI;
	PreviewUI previewUI;

	bool isLooping;
	bool isFall;
	int score;
	int lines;
	int speed;

public:
	GameManager()
		: map(new Map), screen(Screen::GetInstance()), input(Input::GetInstance()), activeBlock(new Block), nextBlock(new Block), 
		isFall(false), isLooping(true), score(0), lines(0), speed(150)
	{
		this->scoreUI = scoreUI;
		this->previewUI = previewUI;
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
			input->readInputs();
			update();
			screen->draw(map);
			screen->render();
			Sleep(speed);
		}
	}

	void update() {
		if (isGameOver(fixedBlocks)) {
			isLooping = false;
			Borland::gotoxy(screen->getWidth() + 7, screen->getHeight() - 5);
			printf("GAME OVER");
		}
		scoreUI.drawUI(score, lines, speed);
		scoreUI.printUI(screen->getWidth() + 1);
		previewUI.drawUI(*nextBlock);
		previewUI.printUI(screen->getWidth() + 1, screen->getHeight() - 16);
		createNewBlock();
		moveBlock();
		map->update(*activeBlock, fixedBlocks, score, lines);
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
		if (input->getKeyDown(VK_UP))
		{
			turnBlock(*activeBlock);
		}

		if (input->getKeyDown(VK_LEFT))
		{
			if (checkLeftCollision()) return;
			activeBlock->setPos(activeBlock->getPos().x - 1, activeBlock->getPos().y);
		}

		if (input->getKeyDown(VK_RIGHT))
		{
			if (checkRightCollision()) return;
			activeBlock->setPos(activeBlock->getPos().x + 1, activeBlock->getPos().y);
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

		if (input->getKey(VK_DOWN))
		{
			speed = 50;
		}
		else speed = 150;
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
			if (map->getBoard()[screen->pos2Index(block.getPos().addPos(0, 2))] != ' ') return;

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
			if (map->getBoard()[screen->pos2Index(block.getPos().addPos(2, 0))] != ' ') return;

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

		return false;
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
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(1, 1)) + 1] != ' ') return true;
			else if (map->getBoard()[screen->pos2Index(activeBlock->getPos().addPos(1, 2)) + 1] != ' ') return true;
		}

		return false;
	}

	// 게임 오버 조건 확인
	bool isGameOver(vector<Block*>& fixedBlocks)
	{
		vector<Block*>::iterator it;
		for (it = fixedBlocks.begin(); it != fixedBlocks.end() && !fixedBlocks.empty(); it++)
		{
			if ((*it)->getPos().y - 1 <= 1) {
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
	if (cNumRead == 0) return false;

	for (int i = 0; i < cNumRead; i++)
	{
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode && irInBuf[i].Event.KeyEvent.bKeyDown == TRUE && irInBuf[i].Event.KeyEvent.wRepeatCount == 1) {
			return true;
		}
		return false;
	}
}
bool Input::getKey(WORD virtualKeyCode) {
	if (cNumRead == 0) return false; 

	for (int i = 0; i < cNumRead; i++)
	{
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode && irInBuf[i].Event.KeyEvent.bKeyDown == TRUE && irInBuf[i].Event.KeyEvent.wRepeatCount > 1) {
			return true;
		}
		return false;
	}
}
bool Input::getKeyUp(WORD virtualKeyCode) {
	if (cNumRead == 0) return false;

	for (int i = 0; i < cNumRead; i++)
	{
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode && irInBuf[i].Event.KeyEvent.bKeyDown == FALSE) {
			return true;
		}
		return false;
	}
}