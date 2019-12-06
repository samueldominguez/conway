#pragma once
#include <vector>
#include <iostream>

class cell {
public:
	bool alive;
	int x;
	int y;
	cell(int _x, int _y, bool _al = false) : x{_x}, y{_y}, alive{_al} {}
	cell() : x{-1}, y{-1}, alive{false} {}
};

class conway {
private:
	int rows;
	int cols;
	cell** matrix;
	size_t count_n(int r, int c);
public:
	conway(int r, int c);
	~conway();
	void populate(std::vector<cell> points);
	void step();
	void print(); // for debugging
};
