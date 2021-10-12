#ifndef SCREEN_H
#define SCREEN_H

#define _CRT_SECURE_NO_WARNINGS

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

	Screen(int width, int height); // boundry +2
	virtual ~Screen();

	static Screen* Instance;

public:

	static Screen* GetInstance();
	int getWidth() const;
	void setWidth(int width);
	void clear();
	Position offset2Pos(int offset) const;
	int pos2Index(const Position& pos) const;
	void draw(const Position& pos, const char* shape, const Dimension& dm);
	void render();

};

#endif