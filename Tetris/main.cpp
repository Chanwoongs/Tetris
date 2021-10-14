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

	// constructor (생성자 함수) 메모리공간상에 적재되는 순간 호출되는
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
	void draw(Map* map);
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

//screen
class GameObject {
	Screen* screen;
	Input* input;
public:
	GameObject()
		:screen(Screen::GetInstance()), input(Input::GetInstance())
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
	bool getKeyUp() const {
		return input->getKey(VK_UP);
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
	Block()
		: shapeNum(shapeNum), dm{ 1,1 }, pos{ 7,2 }, isMoving(true)
	{
		srand((unsigned)time(nullptr));
		shapeNum = rand() % 7 + 1;
		if (shapeNum == (int)Shape::I) {
			shape = new char[5];
			strncpy(this->shape, "****", sizeof("****"));
			this->dm = { 1,4 };
		}
		else if (shapeNum == (int)Shape::J) {
			shape = new char[7];
			strncpy(this->shape, "*  ***", sizeof("*  ***"));
			this->dm = { 3,2 };
		}
		else if (shapeNum == (int)Shape::L) {
			shape = new char[7];
			strncpy(this->shape, "  ****", sizeof("  ****"));
			this->dm = { 3,2 };
		}
		else if (shapeNum == (int)Shape::O) {
			shape = new char[5];
			strncpy(this->shape, "****", sizeof("****"));
			this->dm = { 2,2 };
		}
		else if (shapeNum == (int)Shape::S) {
			shape = new char[7];
			strncpy(this->shape, " **** ", sizeof(" **** "));
			this->dm = { 3,2 };
		}
		else if (shapeNum == (int)Shape::T) {
			shape = new char[7];
			strncpy(this->shape, " * ***", sizeof(" * ***"));
			this->dm = { 3,2 };
		}
		else if (shapeNum == (int)Shape::Z) {
			shape = new char[7];
			strncpy(this->shape, "**  **", sizeof("**  **"));
			this->dm = { 3,2 };
		}
	}
	~Block() {
		delete[] shape;
	}
	void turnBlock() {
		Dimension oneXfour{ 1,4 };
		Dimension fourXone{ 4,1 };
		Dimension threeXtwo{ 3,2 };
		Dimension twoXthree{ 2,3 };
		// 1 X 4 -> 4 X 1
		if (dm.comparePos(oneXfour)) {
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
		if (dm.comparePos(fourXone)) {
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
		else if (dm.comparePos(threeXtwo)) {
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
		else if (dm.comparePos(twoXthree)) {
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
	const char* getEmpty4Shape() const {
		return "    ";
	}
	const char* getEmpty6Shape() const {
		return "      ";
	}
	void setIsMoving(bool isMoving) {
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
	void initializeBoard() {
		memset(board, ' ', getSize());
		for (int i = 15; i < 311; i += 15) {
			board[i] = 'l';
		}
		for (int i = 28; i < 314; i += 15) {
			board[i] = 'l';
		}
		for (int i = 0; i < 14; i++) {
			board[i] = '=';
		}
		for (int i = 315; i < 329; i++) {
			board[i] = '=';
		}
		for (int i = 14; i < 315; i += 15) {
			board[i] = '\n';
		}
		board[getSize() - 1] = '\0';
	}
	Position Index2Pos(int offset) const {

	}
	char* getBoard() {
		return board;
	}
	void clear() {
		memset(board, ' ', getSize());
	}
	int pos2Index(const Position& pos) const {
		return (getWidth() + 1) * pos.y + pos.x; // x + 15y
	}
	void drawActiveBlock(Block& block) {
		for (int h = 0; h < block.getDimension().y; h++) {
			if (block.getDimension().comparePos(4, 1) || block.getDimension().comparePos(1, 4) || block.getDimension().comparePos(2, 2)) {
				strncpy(&board[(pos2Index(block.getPos()) + (getWidth() + 1) * h) - 15], &block.getEmpty4Shape()[h * block.getDimension().x], block.getDimension().x);
			}
			else if (block.getDimension().comparePos(2, 3) || block.getDimension().comparePos(3, 2)) {
				strncpy(&board[(pos2Index(block.getPos()) + (getWidth() + 1) * h) - 15], &block.getEmpty6Shape()[h * block.getDimension().x], block.getDimension().x);
			}
		}
		for (int h = 0; h < block.getDimension().y; h++) {
			strncpy(&board[pos2Index(block.getPos()) + (getWidth() + 1) * h], &block.getShape()[h * block.getDimension().x], block.getDimension().x);
		}
	}
	void eraseAfterImage() {
		int i = 0;

		while (board[i] != '*' && (board[i] == '-' || board[i] == 'l'))
		{
			board[i] = ' ';
			i++;
		}
	}
	void eraseLines() {
		for (int h = 1; h < getHeight() - 1; h++) {
			bool isFull = true;
			isFull = checkLinesFull(h);
			if (isFull) {
				moveBlocksDown(h);
			}
		}
	}
	bool checkLinesFull(int h) const {
		for (int i = 1; i < getWidth() - 1; i++) {
			Position pos{ i, h };
			if (board[pos2Index(pos)] != '*') {
				return false;
			}
		}
		return true;
	}
	void moveBlocksDown(int h) {
		for (int i = h - 1; i > 0; i--) {
			for (int j = 1; j < getWidth() - 1; j++)
			{
				Position pos2{ j, i };
				board[pos2Index(pos2) + 15] = board[pos2Index(pos2)];
			}
		}
	}
	void freezeBlock(Block& block) {
		// shape ㅣ
		if (block.getShapeNum() == 1 && block.getDimension().comparePos(1,4)) {
			if (board[pos2Index(block.getPos().addPos(0,4))] != ' ') {
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 1 && block.getDimension().comparePos(4, 1)) {
			if (board[pos2Index(block.getPos().addPos(0,1))] != ' ' || board[pos2Index(block.getPos().addPos(1,1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ' || board[pos2Index(block.getPos().addPos(3, 1))] != ' ') {
				block.setIsMoving(false);
			}
		}
		// shape J
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0,3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0,3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos().addPos(0,1))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0,1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 2 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos().addPos(0, 1))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		// shape L
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1,0))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 3 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 1))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		// shape O
		else if (block.getShapeNum() == 4) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		// shape S
		else if (block.getShapeNum() == 5 && block.getDimension().comparePos(2, 3)) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 5 && block.getDimension().comparePos(3, 2)) {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		// shape T
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(3, 2) && board[pos2Index(block.getPos())] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 1))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] == ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 2))] != ' ' || board[pos2Index(block.getPos().addPos(1, 3))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 6 && block.getDimension().comparePos(2, 3) && board[pos2Index(block.getPos().addPos(1, 0))] != ' ') {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		// shape Z
		else if (block.getShapeNum() == 7 && block.getDimension().comparePos(2, 3)) {
			if (board[pos2Index(block.getPos().addPos(0, 3))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
		else if (block.getShapeNum() == 7 && block.getDimension().comparePos(3, 2)) {
			if (board[pos2Index(block.getPos().addPos(0, 1))] != ' ' || board[pos2Index(block.getPos().addPos(1, 2))] != ' ' || board[pos2Index(block.getPos().addPos(2, 2))] != ' ')
			{
				block.setIsMoving(false);
			}
		}
	}
	void update(Block& block) {
		drawActiveBlock(block);
		eraseAfterImage();
		eraseLines();
		freezeBlock(block);
	}
	bool gameOver() {
	}
};

class GameManager {
	Map* map;
	Screen* screen;
	Input* input;
	Block* activeBlock;
	Block* nextBlock;



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
		map->update(*activeBlock);
		moveBlock();
	}
	void createNewBlock() {
		if (activeBlock->getIsMoving()) return;
		activeBlock = nextBlock;
		nextBlock = new Block;
	}
	void moveBlock() {
		if (!activeBlock->getIsMoving()) return;
		activeBlock->setPos(activeBlock->getPos().x, activeBlock->getPos().y + 1);
		if (input->getKeyUp(VK_UP)) {
			activeBlock->turnBlock();
		}
		if (input->getKeyUp(VK_LEFT)) {
			activeBlock->setPos(activeBlock->getPos().x - 1, activeBlock->getPos().y);
		}
		if (input->getKeyUp(VK_RIGHT)) {
			activeBlock->setPos(activeBlock->getPos().x + 1, activeBlock->getPos().y);
		}
		if (input->getKeyUp(VK_DOWN)) {
			activeBlock->turnBlock();
		}
		if (input->getKeyUp(VK_UP)) {
			activeBlock->turnBlock();
		}
	}
};

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