#include "game.hpp"

// Returns true if successful, false otherwise
bool game::initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
		return false;
	}
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		std::cout << "Warning: Linear texture filtering not enabled" << std::endl;
		return false;
	}
	win = SDL_CreateWindow(win_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, s_width, s_height, SDL_WINDOW_SHOWN);
	if (win == NULL) {
		std::cout << "Window could not be created: " << SDL_GetError() << std::endl;
		return false;
	}
	rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (rend == NULL) {
		std::cout << "Renderer could not be created: " << SDL_GetError() << std::endl;
		return false;
	}
	// set render draw color
	SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);

	// SDL_IMage
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		std::cout << "SDL_Image could not be initialized: " << SDL_GetError() << std::endl;
		return false;
	}

	// success
	return true;
}

game::game(int width, int height)
{
	s_width = width;
	s_height = height;
	win = NULL;
	rend = NULL;
}

game::~game()
{
	if (rend) SDL_DestroyRenderer(rend);
	if (win) SDL_DestroyWindow(win);

	IMG_Quit();
	SDL_Quit();
}

void game::game_loop()
{
	bool quit = false;
	SDL_Event ev; // event handler
	Uint32 start;

	while (!quit) {
		start = SDL_GetTicks();

		// handle queued events
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT:
				quit = true;
				break;
			}
		}
		// clear screen
		SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(rend);

		// draw line
		SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0xFF, 0xFF);
		SDL_RenderDrawLine(rend, 0, 0, s_width, s_height);

		// update screen
		SDL_RenderPresent(rend);

		// regulate fps
		if (SDL_GetTicks() - start < fpsms)
			SDL_Delay(fpsms - (SDL_GetTicks() - start));
	}
}
