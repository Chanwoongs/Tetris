#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <conio.h> 
#include <cstring> 
#include <cstdlib> 
#include <string> 
#include <Windows.h>
#include "Utils.h"

class Screen {
	int	width; 
	int	height;
	int	size;
	char* canvas;

	Screen(int width = 12 + 2, int height = 20 + 2) // boundry +2
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
	virtual ~Screen()
	{
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
	int getWidth() const
	{
		return width;
	}
	void setWidth(int width)
	{
		this->width = width;
	}
	void clear()
	{
		memset(canvas, ' ', size);
	}
	Position offset2Pos(int offset) const {

	}
	int pos2Offset(const Position& pos) const {
		return (width + 1) * pos.y + pos.x;
	}
	void draw(const Position& pos, const char* shape, const Dimension& sz = Position{ 1, 1 })
	{
		int offset = pos2Offset(pos);
		for (int h = 0; h < sz.y; h++)
			strncpy(&canvas[offset + (width + 1) * h], &shape[h * sz.x], sz.x);
	}
	void render()
	{
		Borland::gotoxy(0, 0);
		for (int h = 0; h < height; h++)
			canvas[(width + 1) * (h + 1) - 1] = '\n';
		canvas[size - 1] = '\0';
		printf("%s", canvas);
	}

};

#endif