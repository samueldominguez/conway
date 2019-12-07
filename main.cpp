#include <iostream>

#include "game.hpp"

int main()
{
	game g(640, 480, 300, 300);
	g.initialize();
	g.game_loop();
	return 0;
}
