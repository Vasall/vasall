void menu_upd()
{
	XSDL_Event event;

	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
	/* that occurs.                                                  */
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT)
			running = 0;

		/* Process interactions with the UI */
		if(XSDL_ProcEvent(context, &event) > -1)
			continue;
	}
}

int ox = 0, oy = 0;

void game_upd()
{
	ox = (ox + 2) % 80;
	oy = (oy + 2) % 80;

	SDL_SetRenderDrawColor(renderer, 
			255, 
			0, 
			0, 
			255);

	int x, y;
	for(x = ox; x < context->win_w; x += 80) {
		SDL_RenderDrawLine(renderer, 
				x, 0, x, context->win_h);

		for(y = oy; y < context->win_h; y += 80) {
			SDL_RenderDrawLine(renderer, 
					0, y, context->win_w, y);
		}
	}
}
