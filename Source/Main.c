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
#include "Menu.h"
#include "Questions.h"
#include "SDL2/SDL_ttf.h"
#include "Init.h"
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../Libraries/json.h" // TODO:  _very_ unhappy about this

#define ever ;; // hehehe

enum [[clang::enum_extensibility(closed)]] mode {
	menu,
	questions
};


int main([[maybe_unused]] int argc, [[maybe_unused]] const char ** argv)
{
	/* I'm aware that `rand()` is a terrible method of randomness, much as i am also aware that this is a terrible method of seeding the pool.  However, when i tried to use `/dev/random` or `/dev/urandom` (via syscall or file!) instead of `rand()`, it would _always_ completely and entirely block the entire program. */
	srand((unsigned int)time(NULL));
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	atexit(SDL_Quit);
	TTF_Init();
	atexit(TTF_Quit);

	struct assptrs assptrs = load_assptrs();

	SDL_Window * window = SDL_CreateWindow("ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenwidth, screenheight, 0);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);

	enum mode mode = menu;
	struct timespec firsttime, secondtime;
	SDL_Surface * screen = SDL_CreateRGBSurfaceWithFormat(0, screenwidth, screenheight, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_SetSurfaceRLE(screen, true);
	struct extra extra = {0};
	extra.filereq = -1;
	screen->userdata = &extra;
	SDL_Texture * screentex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screenwidth, screenheight);
	void * pixels; int pitch;
	for (ever) {
		timespec_get(&firsttime, TIME_UTC); // For throttling FPS to reduce resource usage — see end of loop.

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
		SDL_RenderClear(renderer);
		switch (mode) {
		case menu:
			menu_render(screen, assptrs);
			break;
		case questions:
			questions_render(screen, assptrs);
			break;
		}
		SDL_LockSurface(screen);
		SDL_LockTexture(screentex, NULL, &pixels, &pitch);
		memmove(pixels, screen->pixels, pitch * screenheight * 8 / CHAR_BIT);
		SDL_UnlockTexture(screentex);
		SDL_UnlockSurface(screen);
		SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0x00, 0x00, 0x00, 0xFF));
		SDL_RenderCopy(renderer, screentex, NULL, NULL);
		SDL_RenderPresent(renderer);

		if (((struct extra *)screen->userdata)->filereq >= 0) {
			if (assptrs.curfile != NULL)
				json_value_free(assptrs.curfile);
			char fname[0b10'0000'0000] = "./Assets/";
			strcat(fname, assptrs.filenames[((struct extra *)screen->userdata)->filereq]);
			strcat(fname, ".json");
			struct stat fstatus = {0};
			stat(fname, &fstatus);
			FILE * file = fopen(fname, "r");
			char * buf = malloc(fstatus.st_size);
			fread(buf, fstatus.st_size, 1, file);
			fclose(file);
			assptrs.curfile = json_parse(buf, fstatus.st_size);
			free(buf);
		} else {
			if (assptrs.curfile != NULL)
				json_value_free(assptrs.curfile);
		}
		if (((struct extra *)(screen->userdata))->quit) break;
		if (((struct extra *)(screen->userdata))->swap) {
			mode = (mode == menu) ? questions : menu;
			((struct extra *)(screen->userdata))->swap = false;
		}

		timespec_get(&secondtime, TIME_UTC);
		secondtime.tv_nsec -= firsttime.tv_nsec; secondtime.tv_sec -= firsttime.tv_sec;
		if (secondtime.tv_sec > 0) continue;
		if (secondtime.tv_nsec > 27777777) continue;
		firsttime.tv_sec = 0;
		firsttime.tv_nsec = 27777777 - secondtime.tv_nsec;
		nanosleep(&firsttime, NULL);
	}

	SDL_DestroyTexture(screentex);
	SDL_FreeSurface(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	unload_assptrs(assptrs);
	TTF_Quit();
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_Quit();
	return 0;
}
