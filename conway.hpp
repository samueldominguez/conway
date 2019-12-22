#pragma once
#include <vector>
#include <random>
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
	std::random_device rd;
	std::mt19937 eng;
	std::uniform_int_distribution<> distr; // inclusive
public:
	cell** matrix;
	conway(int r, int c) : rows{r}, cols{c}, matrix{NULL}, populated{false}, eng{rd()}, distr{0, 1} {}
	~conway();
	void allocate();
	void populate(std::vector<cell> points);
	void populate_rand();
	void step();
	void print(); // for debugging
};
