#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <memory>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

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
	// font
	TTF_Font *gfont;
	// get how many cells we can show on screen
	// across each dimension at a given time
	int y_con_cells() { return conway_vp.h / (def_cell_size * cell_scale); }
	int x_con_cells() { return conway_vp.w / (def_cell_size * cell_scale); }
	bool within_rect(SDL_Rect& r, int x, int y);
	SDL_Texture* load_text(std::string text, SDL_Color color);
public:
	game(int width, int height, int x_grid, int y_grid);
	~game();
	bool initialize(); // initialize SDL and other subsystems
	void game_loop();
};

class button {
public:
	static constexpr int anim_offset = 5;
	SDL_Rect dest;
	SDL_Texture* texture;
	SDL_Color color;
	Uint32 timer;
	button(SDL_Texture* t, int x = 0, int y = 0, SDL_Color bcol = {0, 0, 0}) : texture{t}, color{bcol} {
		// set width and height of texture
		int w, h;
		dest.x = x;
		dest.y = y;
		SDL_QueryTexture(t, NULL, NULL, &w, &h);
		dest.w = w;
		dest.h = h;
	}
	void time() { timer = SDL_GetTicks(); }
	Uint32 elapsed() { return SDL_GetTicks() - timer; }
	// center and map button
	void center_y(const SDL_Rect& vp) {
		dest.y = (vp.h / 2) - (dest.h / 2);
	}
	void map_x(const SDL_Rect& vp, int cell, int div) {
		int c_size = vp.w / div;
		dest.x = (cell * c_size) - (c_size / 2) - (dest.w / 2);
	}
	// animation methods
	void press_anim() {
		dest.y += anim_offset;
	}
	void depress_anim() {
		dest.y -= anim_offset;
	}
	// draw method
	void draw(SDL_Renderer* rend) {
		SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, 0xFF);
		SDL_RenderDrawRect(rend, &dest);
		SDL_RenderCopy(rend, texture, nullptr, &dest);
	}
};
