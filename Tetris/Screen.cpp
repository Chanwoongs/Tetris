#include "Screen.h"

Screen::Screen(int width, int height) // boundry +2
	: width(width), height(height), canvas(new char[(width + 1) * height])
{
	bool faultyInput = false;
	if (this->width <= 0) {
		this->width = 10;
		faultyInput = true;
	}
	if (this->height <= 0) {
		this->height = 10;
		faultyInput = true;
	}
	size = (this->width + 1) * this->height;
	if (faultyInput == true) {
		delete canvas;
		canvas = new char[size];
	}
}
Screen::~Screen()
{
	delete[] canvas;
	canvas = nullptr;
	width = 0; height = 0;
}
Screen* Screen::GetInstance() {
	if (Instance == nullptr) {
		Instance = new Screen;
	}
	return Instance;
}
int Screen::getWidth() const
{
	return width;
}
void Screen::setWidth(int width)
{
	this->width = width;
}
void Screen::clear()
{
	memset(canvas, ' ', size);
}
int Screen::pos2Index(const Position& pos) const {
	return (width + 1) * pos.y + pos.x;
}
void Screen::draw(const Position& pos, const char* shape, const Dimension& sz = Position{ 1, 1 })
{
	int offset = pos2Index(pos);
	for (int h = 0; h < sz.y; h++)
		strncpy(&canvas[offset + (width + 1) * h], &shape[h * sz.x], sz.x);
}
void Screen::render()
{
	Borland::gotoxy(0, 0);
	for (int h = 0; h < height; h++)
		canvas[(width + 1) * (h + 1) - 1] = '\n';
	canvas[size - 1] = '\0';
	printf("%s", canvas);
}