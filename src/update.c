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


void game_upd()
{
}
