#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

class game {
private:
	//consts
	static constexpr char win_name[] = "Conway's Game of Life";
	static constexpr int fpsms = 100 / 6;
	// dimensions
	int s_width;
	int s_height;
	// window
	SDL_Window* win;
	// renderer
	SDL_Renderer* rend;
public:
	game(int width, int height);
	~game();
	bool initialize(); // initialize SDL and other subsystems
	void game_loop();
};
