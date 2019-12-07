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
	bool populated; // have we populated this?
	int rows;
	int cols;
	size_t count_n(int r, int c);
public:
	cell** matrix;
	conway(int r, int c) : rows{r}, cols{c}, matrix{NULL}, populated{false} {}
	~conway();
	void populate(std::vector<cell> points);
	void step();
	void print(); // for debugging
};
