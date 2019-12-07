#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>

#include "conway.hpp"

class game {
private:
	//consts
	static constexpr char win_name[] = "Conway's Game of Life";
	static constexpr int fpsms = 100 / 6;
	static const int def_cell_size = 10;
	// dimensions
	int s_width;
	int s_height;
	// window
	SDL_Window* win;
	// renderer
	SDL_Renderer* rend;

	// number of cells for conway
	int x_cells;
	int y_cells;
	// conway object
	conway* cw;
	// how often should we update the cells?
	int cell_update_t;
	// viewports
	SDL_Rect conway_vp;
	SDL_Rect gui_vp;
	// cell scale
	float cell_scale;
	// get how many cells we can show on screen
	// across each dimension at a given time
	int y_con_cells() { return conway_vp.h / (def_cell_size * cell_scale); }
	int x_con_cells() { return conway_vp.w / (def_cell_size * cell_scale); }
public:
	game(int width, int height, int x_grid, int y_grid);
	~game();
	bool initialize(); // initialize SDL and other subsystems
	void game_loop();
};
