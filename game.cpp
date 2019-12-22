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
	// SDL_TTF
	if (TTF_Init() == -1) {
		std::cout << "error: SDL_TTF could not initialize: " << TTF_GetError() << std::endl;
		return false;
	}
	gfont = TTF_OpenFont("data/font.ttf", 20);
	if (gfont == NULL) {
		std::cout << "error: failed to load font: " << TTF_GetError() << std::endl;
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
	cell_update_t = 50;
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

	if (gfont) TTF_CloseFont(gfont);
	
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

void game::game_loop()
{
	bool quit = false;
	bool use_random = false; // use random colors?
	bool pause = false; // pause or play?
	bool step = false; // step once?
	SDL_Event ev; // event handler
	Uint32 start, cell_update;

	// random device and generator
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(0x00, 0xFF); // inclusive

	// unique ptrs for text surfaces
	SDL_Color color = {0x00, 0x00, 0x00};
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> pzoom_text(load_text("+ Zoom", color), SDL_DestroyTexture);
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> mzoom_text(load_text("- Zoom", color), SDL_DestroyTexture);
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> pspeed_text(load_text("+ Speed", color), SDL_DestroyTexture);
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> mspeed_text(load_text("- Speed", color), SDL_DestroyTexture);
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> rand_text(load_text("Rand. Col.", color), SDL_DestroyTexture);
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> norm_text(load_text("Norm. Col.", color), SDL_DestroyTexture);
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> pp_text(load_text("Pause/Play", color), SDL_DestroyTexture);
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> step_text(load_text("   Step   ", color), SDL_DestroyTexture);

	// buttons
	button pzoom(pzoom_text.get());
	pzoom.map_y(gui_vp, 1, 2);
	pzoom.map_x(gui_vp, 1, 4);
	button mzoom(mzoom_text.get());
	mzoom.map_y(gui_vp, 2, 2);
	mzoom.map_x(gui_vp, 1, 4);
	button pspeed(pspeed_text.get());
	pspeed.map_y(gui_vp, 1, 2);
	pspeed.map_x(gui_vp, 2, 4);
	button mspeed(mspeed_text.get());
	mspeed.map_y(gui_vp, 2, 2);
	mspeed.map_x(gui_vp, 2, 4);
	button rand_b(rand_text.get());
	rand_b.map_y(gui_vp, 1, 2);
	rand_b.map_x(gui_vp, 3, 4);
	button norm_b(norm_text.get());
	norm_b.map_y(gui_vp, 2, 2);
	norm_b.map_x(gui_vp, 3, 4);
	button pp_b(pp_text.get());
	pp_b.map_y(gui_vp, 1, 2);
	pp_b.map_x(gui_vp, 4, 4);
	button step_b(step_text.get());
	step_b.map_y(gui_vp, 2, 2);
	step_b.map_x(gui_vp, 4, 4);
	
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

	// populate the conway matrix
	cw->populate(p);
	// cw->populate_rand();

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
				if (ev.button.button == SDL_BUTTON_LEFT and
				    within_rect(pzoom.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
					// increase zoom button pressed
					int prev_ycells = y_con_cells();
					int prev_xcells = x_con_cells();
					cell_scale += 0.5;
					c_point.x += (prev_xcells - x_con_cells()) / 2;
					c_point.y += (prev_ycells - y_con_cells()) / 2;
					pzoom.press();
				}  else if (ev.button.button == SDL_BUTTON_LEFT and
					    within_rect(mzoom.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
					// decrease zoom button pressed
					if ((cell_scale - 0.5) <= 0) cell_scale = 0.5;
					else {
						// reshape
						int prev_ycells = y_con_cells();
						int prev_xcells = x_con_cells();
						cell_scale -= 0.5;
						c_point.x += (prev_xcells - x_con_cells()) / 2;
						c_point.y += (prev_ycells - y_con_cells()) / 2;
					}
					mzoom.press();
				} else if (ev.button.button == SDL_BUTTON_LEFT and
					    within_rect(pspeed.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
						// increase speed button pressed
					if (cell_update_t - 50 > 0)
						cell_update_t -= 50;
					pspeed.press();
				} else if (ev.button.button == SDL_BUTTON_LEFT and
					    within_rect(mspeed.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
						// decrease speed button pressed
					cell_update_t += 50;
					mspeed.press();
				} else if (ev.button.button == SDL_BUTTON_LEFT and
					    within_rect(rand_b.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
					// random colors enabled
					use_random = true;
					rand_b.press();
				} else if (ev.button.button == SDL_BUTTON_LEFT and
					    within_rect(norm_b.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
					// normal colors enabled
					use_random = false;
					norm_b.press();
				} else if (ev.button.button == SDL_BUTTON_LEFT and
					  within_rect(pp_b.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
					// pause or play the simulation
					if (pause) pause = false;
					else pause = true;
					pp_b.press();
				} else if (ev.button.button == SDL_BUTTON_LEFT and
					   within_rect(step_b.dest, ev.motion.x, ev.motion.y, &gui_vp)) {
					if (pause) step = true;
					step_b.press();
				}
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			}
		}
		// logic
		Uint32 mouse_s = SDL_GetMouseState(NULL, NULL);
		if (mouse_s & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			// get current mouse coordinates
			int x, y;
			SDL_GetMouseState(&x, &y);
			// map coordinates to location in conway matrix
			cw->matrix[c_point.y + (int)(y / (def_cell_size * cell_scale))][c_point.x + (int)(x / (def_cell_size * cell_scale))].alive = true;
		} else if (mouse_s & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			// map coordinates to location in conway matrix
			cw->matrix[c_point.y + (int)(y / (def_cell_size * cell_scale))][c_point.x + (int)(x / (def_cell_size * cell_scale))].alive = false;
		}
		
		// cell updates every cell_update_t in ms
		if (step == true or (pause == false and SDL_GetTicks() - cell_update >= cell_update_t)) {
			// step conway's game of life
			cw->step();
			// reset the cell_update timer
			cell_update = SDL_GetTicks();
			// always set step to false
			step = false;
		}

		// adjust scale of the cell
		d_cell.w = d_cell.h = def_cell_size * cell_scale;

		// animate butons
		pzoom.anim();
		mzoom.anim();
		pspeed.anim();
		mspeed.anim();
		rand_b.anim();
		norm_b.anim();
		pp_b.anim();
		step_b.anim();
		
		// clear screen
		SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(rend);
		
		// draw conway
		SDL_RenderSetViewport(rend, &conway_vp);
		// SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0xFF, 0xFF);
		for (int y = c_point.y; y < c_point.y + y_con_cells() and y < y_cells; ++y) {
			for (int x = c_point.x; x < c_point.x + x_con_cells() and x < x_cells; ++x) {
				// only draw alive ones, we've cleared the screen already with white
				if (cw->matrix[y][x].alive) {
					if (use_random) SDL_SetRenderDrawColor(rend, distr(eng), distr(eng), distr(eng), 0xFF);
					else SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0x00, 0xFF);
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

		// render buttons
		pzoom.draw(rend);
		mzoom.draw(rend);
		pspeed.draw(rend);
		mspeed.draw(rend);
		rand_b.draw(rend);
		norm_b.draw(rend);
		pp_b.draw(rend);
		step_b.draw(rend);
		
		// update screen
		SDL_RenderPresent(rend);

		// regulate fps
		if (SDL_GetTicks() - start < fpsms)
			SDL_Delay(fpsms - (SDL_GetTicks() - start));
	}
}

bool game::within_rect(SDL_Rect& r, int x, int y, SDL_Rect* vp)
{
	SDL_Rect off = {0, 0, 0, 0};
	if (vp == nullptr) vp = &off;
	if (x >= r.x + vp->x and x <= r.x + vp->x + r.w and
	    y >= r.y + vp->y and y <= r.y + vp->y + r.h) return true;
	else return false;
}

// create texture from text and colour
SDL_Texture* game::load_text(std::string text, SDL_Color color)
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(gfont, text.c_str(), color);
	SDL_Texture* text_texture = nullptr;
	if (text_surface == NULL) {
		std::cout << "error: cannot render text surface: " << TTF_GetError() << std::endl;
		return nullptr;
	} else {
		text_texture = SDL_CreateTextureFromSurface(rend, text_surface);
		if (text_texture == nullptr) {
			std::cout << "error: cannot create texture from surface: " << TTF_GetError() << std::endl;
			return nullptr;
		} else {
			SDL_FreeSurface(text_surface);
			return text_texture;
		}
	}
}
