#include <iostream>

#include "game.hpp"
#include "conway.hpp"

int main()
{
	conway cw(5, 5);
	// how to populate the grid
	std::vector<cell> p;
	// p.push_back(cell(0, 0));
	// p.push_back(cell(1, 1));
	// p.push_back(cell(2, 2));
	// p.push_back(cell(3, 3));
	// p.push_back(cell(4, 4));
	// p.push_back(cell(2, 1));
	// p.push_back(cell(2, 3));
	// populate it
	cw.populate(p);
	cw.print();
	for (int i = 0; i < 5; ++i) {
		std::cout << "Step..." << std::endl;
		cw.step();
		cw.print();
	}
	// game g(640, 480);
	// g.initialize();
	// g.game_loop();
	return 0;
}
