/* LICENSE
 * Copyright © 2022 Blue-Maned_Hawk.  All rights reserved.
 *
 * This project should have come with a file called `LICENSE`.  In the event of any conflict between this comment and that file, that file shall be considered the authority.
 *
 * You may freely use this software.  You may freely distribute this software, so long as you distribute the license and source code with it.  You may freely modify this software and distribute the modifications under a similar license, so long as you distribute the sources with them and you don't claim that they're the original software.  None of this overrides local laws, and if you excercise these rights, you cannot claim that your actions are condoned by the author.
 *
 * This license does not apply to patents or trademarks.
 *
 * This software comes with no warranty, implied or explicit.  The author disclaims any liability for damages caused by this software. */

/* This is the main file for ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls. */

#include <unistd.h>
#include "SDL.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "Menu.h"
#include "Questions.h"

#define ever ;;

enum state_t {
	menu,
	questions
};

int main([[maybe_unused]] int argc, [[maybe_unused]] const char ** argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	atexit(SDL_Quit);

	SDL_Window * window = SDL_CreateWindow("ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_Event e;
	bool quit = false;
	enum state_t state = menu;
	struct timespec firsttime, secondtime;
	for (ever){
		timespec_get(&firsttime, TIME_UTC);

		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT){
				quit++;
				break;
			} else if (state == menu) {
				menu_handle_events(e);
			} else if (state == questions) {
				questions_handle_events(e);
			} else {
				printf("\033[31mThe program has entered an unknown state and will now exit.  Apologies for the inconvenience.");
				exit(EXIT_FAILURE);
			}
		}
		if (quit)
			break;

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
		SDL_RenderClear(renderer);

		switch (state){
		case menu:
			menu_render(renderer);
			break;
		case questions:
			questions_render(renderer);
			break;
		default:
			printf("\033[31mThe program has entered an unknown state and will now exit.  Apologies for the inconvenience.");
			exit(EXIT_FAILURE);
		}

		SDL_RenderPresent(renderer);

		timespec_get(&secondtime, TIME_UTC);
		secondtime.tv_nsec -= firsttime.tv_nsec; secondtime.tv_sec -= firsttime.tv_sec;
		if (secondtime.tv_sec > 0)
			continue;
		if (secondtime.tv_nsec > 27000000)
			continue;
		firsttime.tv_sec = 0;
		firsttime.tv_nsec = 27000000 - secondtime.tv_nsec;
		nanosleep(&firsttime, NULL);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_Quit();
	return 0;
}
