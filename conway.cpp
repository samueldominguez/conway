#include "conway.hpp"

void conway::populate(std::vector<cell> points)
{
	// allocate
	matrix = new cell*[rows];
	for (int i = 0; i < rows; ++i) {
		matrix[i] = new cell[cols];
	}
	// populate
	for (auto& p : points) {
		matrix[p.y][p.x].alive = true;
	}
	populated = true;
}

conway::~conway()
{
	if (!populated) return;
	for (int r = 0; r < rows; ++r) {
		delete[] matrix[r];
	}
	delete[] matrix;
}

// conway's game of life rules:
// 1. any live cell with two or three n. survives
// 2. any dead cell with three live n. is born
// 3. all other live/dead cells die/remain dead
void conway::step()
{
	std::vector<cell> updates;
	// record updates without changing current state
	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < cols; ++c) {
			int n = count_n(r, c);
			if (matrix[r][c].alive and (n < 2 or n > 3)) {
				cell t = matrix[r][c];
				t.alive = false;
				t.y = r;
				t.x = c;
				updates.push_back(t);
			} else if ((!matrix[r][c].alive) and n == 3) {
				cell t = matrix[r][c];
				t.alive = true;
				t.y = r;
				t.x = c;
				updates.push_back(t);
			}
		}
	}
	// update all cells that changed
	for (auto& c : updates) matrix[c.y][c.x].alive = c.alive;
}

size_t conway::count_n(int r, int c)
{
	size_t n = 0;
	bool cmin = c - 1 > 0;
	bool cmax = c + 1 < cols;
	bool rmin = r - 1 > 0;
	bool rmax = r + 1 < rows;
	// top row
	if (rmin) {
		if (matrix[r - 1][c].alive) n++;
		if (cmin and matrix[r - 1][c - 1].alive) n++;
		if (cmax and matrix[r - 1][c + 1].alive) n++;
	}
	// middle row
	if (cmin and matrix[r][c - 1].alive) n++;
	if (cmax and matrix[r][c + 1].alive) n++;
	// bot row
	if (rmax) {
		if (matrix[r + 1][c].alive) n++;
		if (cmin and matrix[r + 1][c - 1].alive) n++;
		if (cmax and matrix[r + 1][c + 1].alive) n++;
	}
	return n;
}

void conway::print()
{
	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < cols; ++c) {
			int n = 0;
			if (matrix[r][c].alive) n = 1;
			std::cout << n << " ";
		}
		std::cout << std::endl;
	}
}
