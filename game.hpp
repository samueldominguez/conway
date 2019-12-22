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
	bool within_rect(SDL_Rect& r, int x, int y, SDL_Rect* vp = nullptr);
	SDL_Texture* load_text(std::string text, SDL_Color color);
public:
	game(int width, int height, int x_grid, int y_grid);
	~game();
	bool initialize(); // initialize SDL and other subsystems
	void game_loop();
};

class button {
private:
	bool pressed;
	SDL_Rect text_dest;
public:
	static constexpr int anim_offset = 2;
	static constexpr int ms_dur = 150;
	static constexpr int margin = 4; // 2 pixels of margin on each side
	SDL_Rect dest;
	SDL_Texture* texture;
	SDL_Color color;
	Uint32 timer;
	button(SDL_Texture* t, int x = 0, int y = 0, SDL_Color bcol = {0, 0, 0}) : texture{t}, color{bcol}, pressed{false} {
		// set width and height of texture
		int w, h;
		SDL_QueryTexture(t, NULL, NULL, &w, &h);
		// texture destination
		text_dest.x = x;
		text_dest.y = y;
		text_dest.w = w;
		text_dest.h = h;
		// button destination
		dest.x = x;
		dest.y = y;
		dest.w = w + margin; // add some margin
		dest.h = h + margin; // ==
	}
	void time() { timer = SDL_GetTicks(); }
	Uint32 elapsed() { return SDL_GetTicks() - timer; }
	// center and map button
	void center_y(const SDL_Rect& vp) {
		dest.y = (vp.h / 2) - (dest.h / 2);
		text_dest.y = dest.y + (margin / 2);
	}
	void map_x(const SDL_Rect& vp, int cell, int div) {
		int c_size = vp.w / div;
		dest.x = (cell * c_size) - (c_size / 2) - (dest.w / 2);
		text_dest.x = dest.x + (margin / 2);
	}
	void map_y(const SDL_Rect& vp, int cell, int div) {
		int c_size = vp.h / div;
		dest.y = (cell * c_size) - (c_size / 2) - (dest.h / 2);
		text_dest.y = dest.y + (margin / 2);
	}
	// animation methods
	void anim() {
		if (pressed == false) return;
		else if (elapsed() < ms_dur) return;
		else {
			pressed = false;
			depress_anim();
		}
	}
	void press() {
		if (pressed == true) return;
		else {
			pressed = true;
			time();
			press_anim();
		}
	}
	void press_anim() {
		dest.y += anim_offset;
		text_dest.y = dest.y + (margin / 2);
	}
	void depress_anim() {
		dest.y -= anim_offset;
		text_dest.y = dest.y + (margin / 2);
	}
	// draw method
	void draw(SDL_Renderer* rend) {
		SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, 0xFF);
		SDL_RenderDrawRect(rend, &dest);
		draw_depth_outline(rend, 2);
		SDL_RenderCopy(rend, texture, nullptr, &text_dest);
	}
	void draw_depth_outline(SDL_Renderer* rend, int layers = 1) {
		// draw missing pixel from SDL_RenderDrawRect()
		SDL_RenderDrawPoint(rend, dest.x + dest.w - 1, dest.y + dest.h - 1);
		for (int i = 0; i < layers; ++i) {
			SDL_RenderDrawLine(rend, dest.x, dest.y + dest.h + i, dest.x + dest.w + i, dest.y + dest.h + i);
			SDL_RenderDrawLine(rend, dest.x + dest.w + i, dest.y, dest.x + dest.w + i, dest.y + dest.h + i);
		}
	}
};
