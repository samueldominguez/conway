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

	// SDL_Image
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		std::cout << "SDL_Image could not be initialized: " << SDL_GetError() << std::endl;
		return false;
	}

	// success
	return true;
}

game::game(int width, int height, int x_grid, int y_grid)
{
	s_width = width;
	s_height = height;
	win = NULL;
	rend = NULL;
	// cell update threshold to 1s by default, in ms
	cell_update_t = 100;
	// viewports: 7/8ths of height for the conway rendering
	//            1/8ths of height for the gui
	// cell: 10px by default although can scale
	conway_vp = {0, 0, s_width, (int)std::lround(s_height * (7/8.0))};
	gui_vp = {0, (int)std::lround(s_height * (7/8.0)), s_width, (int)std::lround(s_height * (1/8.0))};
	// cell scale
	cell_scale = 1;
	// cell bounds
	x_cells = x_grid;
	y_cells = y_grid;
	// initialize conway
	cw = new conway(y_cells, x_cells);
}

game::~game()
{
	if (rend) SDL_DestroyRenderer(rend);
	if (win) SDL_DestroyWindow(win);
	if (cw) delete cw;

	IMG_Quit();
	SDL_Quit();
}

void game::game_loop()
{
	bool quit = false;
	bool insert_cells = false; // insert cells at mouse point?
	SDL_Event ev; // event handler
	Uint32 start, cell_update;

	// cell
	SDL_Rect d_cell;

	// center point
	SDL_Rect c_point;
	if (x_con_cells() >= x_cells) c_point.x = 0;
	else c_point.x = (x_cells / 2.0) - (x_con_cells() / 2.0);
	if (y_con_cells() >= y_cells) c_point.y = 0;
	else c_point.y = (y_cells / 2.0) - (y_con_cells() / 2.0);

	SDL_Rect gui = {0, 0, gui_vp.w, gui_vp.h};
	// debug
	std::vector<cell> p;
	p.push_back(cell(150, 150));
	p.push_back(cell(152, 150));
	p.push_back(cell(152, 149));
	p.push_back(cell(154, 148));
	p.push_back(cell(154, 147));
	p.push_back(cell(154, 146));
	p.push_back(cell(156, 147));
	p.push_back(cell(156, 146));
	p.push_back(cell(157, 146));
	p.push_back(cell(156, 145));
	p.push_back(cell(150, 158));
	p.push_back(cell(151, 158));
	p.push_back(cell(152, 158));
	p.push_back(cell(152, 157));
	p.push_back(cell(151, 156));

	// p.push_back(cell(1, 2));
	// p.push_back(cell(2, 2));
	// p.push_back(cell(3, 2));
	
	// populate the conway matrix
	cw->populate(p);

	// get access to the keyboard state
	const Uint8* key_state = SDL_GetKeyboardState(NULL);
	// start logic timers
	cell_update = SDL_GetTicks();
	while (!quit) {
		start = SDL_GetTicks();

		// handle queued events
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				// scrolling around the map
				if (key_state[SDL_SCANCODE_UP]) {
					if (c_point.y  > 0)
						c_point.y--;
				}
				if (key_state[SDL_SCANCODE_DOWN]) {
					if (c_point.y + y_con_cells() < y_cells)
						c_point.y++;
				}
				if (key_state[SDL_SCANCODE_RIGHT]) {
					if (c_point.x + x_con_cells() < x_cells)
						c_point.x++;
				}
				if (key_state[SDL_SCANCODE_LEFT]) {
					if (c_point.x > 0)
						c_point.x--;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				// placing new cells with mouse
				if (ev.button.button == SDL_BUTTON_LEFT and
				    within_rect(conway_vp, ev.motion.x, ev.motion.y))
					insert_cells = true;
				break;
			case SDL_MOUSEBUTTONUP:
				// no longer placing new cells with mouse
				if (ev.button.button == SDL_BUTTON_LEFT) insert_cells = false;
				break;
			}
		}
		// logic
		if (insert_cells) {
			// get current mouse coordinates
			int x, y;
			SDL_GetMouseState(&x, &y);
			// map coordinates to location in conway matrix
			cw->matrix[c_point.y + (int)(y / (def_cell_size * cell_scale))][c_point.x + (int)(x / (def_cell_size * cell_scale))].alive = true;
		}
		
		// cell updates every 1s
		if (SDL_GetTicks() - cell_update >= cell_update_t) {
			// step conway's game of life
			cw->step();
			// reset the cell_update timer
			cell_update = SDL_GetTicks();
		}

		// adjust scale of the cell
		d_cell.w = d_cell.h = def_cell_size * cell_scale;
		
		// clear screen
		SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(rend);
		
		// draw conway
		SDL_RenderSetViewport(rend, &conway_vp);
		SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0xFF, 0xFF);

		for (int y = c_point.y; y < c_point.y + y_con_cells() and y < y_cells; ++y) {
			for (int x = c_point.x; x < c_point.x + x_con_cells() and x < x_cells; ++x) {
				// only draw alive ones, we've cleared the screen already with white
				if (cw->matrix[y][x].alive) {
					SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0x00, 0xFF);
					d_cell.x = (x - c_point.x) * d_cell.w;
					d_cell.y = (y - c_point.y) * d_cell.h;
					SDL_RenderFillRect(rend, &d_cell);
				}
			}
		}

		// draw gui
		SDL_RenderSetViewport(rend, &gui_vp);
		SDL_SetRenderDrawColor(rend, 0xA9, 0xA9, 0xA9, 0xFF);
		SDL_RenderFillRect(rend, &gui);
		// update screen
		SDL_RenderPresent(rend);

		// regulate fps
		if (SDL_GetTicks() - start < fpsms)
			SDL_Delay(fpsms - (SDL_GetTicks() - start));
	}
}

bool game::within_rect(SDL_Rect& r, int x, int y)
{
	if (x >= r.x and x <= r.x + r.w and
	    y >= r.y and y <= r.y + r.h) return true;
	else return false;
}
