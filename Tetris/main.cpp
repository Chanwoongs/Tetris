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
		: shape(shape), shapeNum(shapeNum), dm{ 1,1 }, pos{ getWidth() / 2,2 }, isMoving(true)
	{
		srand((unsigned)time(nullptr));

		// shape 랜덤 선정
		shapeNum = rand() % 7 + 1;

		// shape과 dm 초기화
		// I Block
		if (shapeNum == (int)Shape::I) {
			shape = new char[5];
			strncpy(this->shape, "****", sizeof("****"));
			this->dm = { 1,4 };
		}
		// J Block
		else if (shapeNum == (int)Shape::J) {
			shape = new char[7];
			strncpy(this->shape, "*  ***", sizeof("*  ***"));
			this->dm = { 3,2 };
		}
		// L Block
		else if (shapeNum == (int)Shape::L) {
			shape = new char[7];
			strncpy(this->shape, "  ****", sizeof("  ****"));
			this->dm = { 3,2 };
		}
		// O Block
		else if (shapeNum == (int)Shape::O) {
			shape = new char[5];
			strncpy(this->shape, "****", sizeof("****"));
			this->dm = { 2,2 };
		}
		// S Block
		else if (shapeNum == (int)Shape::S) {
			shape = new char[7];
			strncpy(this->shape, " **** ", sizeof(" **** "));
			this->dm = { 3,2 };
		}
		// T Block
		else if (shapeNum == (int)Shape::T) {
			shape = new char[7];
			strncpy(this->shape, " * ***", sizeof(" * ***"));
			this->dm = { 3,2 };
		}
		// Z Block
		else if (shapeNum == (int)Shape::Z) {
			shape = new char[7];
			strncpy(this->shape, "**  **", sizeof("**  **"));
			this->dm = { 3,2 };
		}
	}
	~Block() {
		delete[] shape;
	}

	// 블럭 반시계 방향으로 돌리기
	void turnBlock() {
		Dimension oneXfour{ 1,4 };
		Dimension fourXone{ 4,1 };
		Dimension threeXtwo{ 3,2 };
		Dimension twoXthree{ 2,3 };
		// 1 X 4 -> 4 X 1
		if (dm.comparePos(oneXfour)) 
		{
			char temp[4] = { ' ' };
			temp[0] = this->shape[3];
			temp[1] = this->shape[2];
			temp[2] = this->shape[1];
			temp[3] = this->shape[0];

			for (int i = 0; i < 4; i++)
			{
				this->shape[i] = temp[i];
			}
			this->dm = fourXone;
		}
		// 4 X 1 -> 1 X 4
		if (dm.comparePos(fourXone)) 
		{
			char temp[4] = { ' ' };
			temp[0] = this->shape[3];
			temp[1] = this->shape[2];
			temp[2] = this->shape[1];
			temp[3] = this->shape[0];

			for (int i = 0; i < 4; i++)
			{
				this->shape[i] = temp[i];
			}
			this->dm = oneXfour;
		}
		// 3 X 2 -> 2 X 3
		else if (dm.comparePos(threeXtwo)) 
		{
			char temp[6] = { ' ' };
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
			this->dm = twoXthree;
		}
		// 2 X 3 -> 3 X 2
		else if (dm.comparePos(twoXthree)) 
		{
			char temp[6] = { ' ' };
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
			this->dm = threeXtwo;
		}
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

	Dimension getDimension() const {
		return dm;
	}

	const char* getShape() const{
		return shape;
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
			board[i] = 'l';
		}
		for (int i = 28; i < 314; i += 15) 
		{
			board[i] = 'l';
		}
		for (int i = 0; i < 14; i++) 
		{
			board[i] = '=';
		}
		for (int i = 315; i < 329; i++) 
		{
			board[i] = '=';
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

	// Position을 Index로 변환
	int pos2Index(const Position& pos) const 
	{
		return (getWidth() + 1) * pos.y + pos.x; // x + 15y
	}

	// ActiveBlock 그리기
	void drawBlock(Block& block) {
		if (block.getIsMoving()) 
		{
			// \0 전까지만 그린다
			for (int h = 0; h < block.getDimension().y; h++)
			{
				strncpy(&board[pos2Index(block.getPos()) + (getWidth() + 1) * h], &block.getShape()[h * block.getDimension().x], block.getDimension().x);
			}
		}
	}

	// board에 남아있는 Block 그리기
	void drawFixedBlocks(vector<Block*> &fixedBlocks) {
		//매니저에서 고정 블럭들의 shape 받아서 draw
		vector<Block*>::iterator it;
		for(it = fixedBlocks.begin(); it != fixedBlocks.end() && !fixedBlocks.empty(); it++)
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
			for (int h = 0; h < (*it)->getDimension().y; h++)
			{
				strncpy(&board[pos2Index((*it)->getPos()) + (getWidth() + 1) * h], &((*it)->getShape()[h * (*it)->getDimension().x]), (*it)->getDimension().x);
			}
		} 
	}

	// 줄 지우기
	void eraseLines() 
	{
		for (int h = 1; h < getHeight() - 1; h++) 
		{
			bool isFull = true;
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

			if (board[pos2Index(pos)] != '*') 
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
		if (block.getShapeNum() == 1 && block.getDimension().comparePos(1,4)) {
			if (board[pos2Index(block.getPos().addPos(0,4))] != ' ') {
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 1 && block.getDimension().comparePos(4, 1)) {
			if (board[pos2Index(block.getPos().addPos(0,1))] != ' ' || board[pos2Index(block.getPos().addPos(1,1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ' || board[pos2Index(block.getPos().addPos(3, 1))] != ' ') {
				block.setMovingFlag(false);
			}
		}
		// shape J
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0,3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0,3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos().addPos(0,1))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0,1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos().addPos(0, 1))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape L
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1,0))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape O
		else if (block.getShapeNum() == 4) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape S
		else if (block.getShapeNum() == 5 && block.getDimension().comparePos(2, 3)) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 5 && block.getDimension().comparePos(3, 2)) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape T
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		// shape Z
		else if (block.getShapeNum() == 7 && block.getDimension().comparePos(2, 3)) {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
		else if (block.getShapeNum() == 7 && block.getDimension().comparePos(3, 2)) {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setMovingFlag(false);
			}
		}
	}

	void update(Block& block, vector<Block*>& fixedBlocks)
	{
		initializeBoard();
		drawFixedBlocks(fixedBlocks);
		drawBlock(block); // activeBlock draw
		//eraseLines();
		freezeBlock(block);
	}

	// 게임 오버
	bool gameOver() {
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
		: map(new Map), screen(Screen::GetInstance()),  input(Input::GetInstance()), activeBlock(new Block), nextBlock(new Block), isFall(false), isLooping(true)
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
			input->readInputs();
			update();
			screen->draw(map);
			screen->render();
			Sleep(100);
		}
	}

	void update() {
		createNewBlock();
		map->update(*activeBlock, fixedBlocks);
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

		activeBlock->setPos(activeBlock->getPos().x, activeBlock->getPos().y + 1);
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
	if (cNumRead == 0) return false; // 현재 콘솔에서 읽은 갯수가 0이냐?

	for (int i = 0; i < cNumRead; i++)
	{
		bool b = false;
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (!b)	{
			if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode && irInBuf[i].Event.KeyEvent.bKeyDown == TRUE) {
				b = true;
				return true;
			}
		}
		
		return false;
	}
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