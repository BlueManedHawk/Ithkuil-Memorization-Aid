/* LICENSE
 *
 * Copyright © 2022, 2023 Blue-Maned_Hawk. All rights reserved.
 *
 * You may freely use this work for any purpose, to the extent permitted by law. You may freely make this work available to others by any means, to the extent permitted by law. You may freely modify this work in any way, to the extent permitted by law. You may freely make works derived from this work available to others by any means, to the extent permitted by law.
 *
 * Should you choose to exercise any of these rights, you must give clear and conspicuous attribution to the original author, and you must not make it seem in any way like the author condones your act of exercising these rights in any way.
 *
 * Should you choose to exercise the second right listed above, you must make this license clearly and conspicuously available along with the original work, and you must clearly and conspicuously make the information necessary to reconstruct the work available along with the work.
 *
 * Should you choose to exercise the fourth right listed above, you must put any derived works you construct under a license that grants the same rights as this one under the same conditions and with the same restrictions, you must clearly and conspicuously make that license available alongside the work, you must clearly and conspicuously make the information necessary to reconstruct the work available alongside the work, you must clearly and conspicuously describe the changes which have been made from the original work, and you must not make it seem in any way like your derived works are the original work in any way.
 *
 * This license only applies to the copyright of this work, and does not apply to any other intellectual property rights, including but not limited to patent and trademark rights.
 *
 * THIS WORK COMES WITH ABSOLUTELY NO WARRANTY OF ANY KIND, IMPLIED OR EXPLICIT. THE AUTHOR DISCLAIMS ANY LIABILITY FOR ANY DAMAGES OF ANY KIND CAUSED DIRECTLY OR INDIRECTLY BY THIS WORK. */

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
#include "../Libraries/mjson.h"
#include <signal.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] const char ** argv)
{
	/* I'm aware that `rand()` is a terrible method of randomness, much as i am also aware that this is a terrible method of seeding the pool.  However, when i tried to use `/dev/random` or `/dev/urandom` (via syscall or file!) instead of `rand()`, it would _always_ completely and entirely block the entire program. */
	srand((unsigned int)time(NULL));
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	atexit(SDL_Quit);
	TTF_Init();
	atexit(TTF_Quit);

	struct assptrs assptrs = load_assptrs();

	signal(SIGALRM, alarm_handler);

	SDL_Window * window = SDL_CreateWindow("ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenwidth, screenheight, SDL_WINDOW_RESIZABLE);  int w, h;
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);

	bool inmenu = true;
	struct timespec firsttime, secondtime;
	SDL_Surface * screen = SDL_CreateRGBSurfaceWithFormat(0, screenwidth, screenheight, 32, SDL_PIXELFORMAT_RGBA32);  SDL_SetSurfaceRLE(screen, true);
	struct extra extra = {0};  extra.filereq = -1;
	SDL_Texture * screentex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screenwidth, screenheight);
	void * pixels;  int pitch;
#define ever ;;
	for (ever) {
#undef ever
		timespec_get(&firsttime, TIME_UTC); // For throttling FPS to reduce resource usage — see end of loop.

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);
		if (inmenu)
			menu_render(screen, assptrs, &extra, window);
		else
			questions_render(screen, assptrs, &extra, window);
		SDL_LockSurface(screen);
		SDL_LockTexture(screentex, NULL, &pixels, &pitch);
		memmove(pixels, screen->pixels, pitch * screenheight * 8 / CHAR_BIT);
		SDL_UnlockTexture(screentex);
		SDL_UnlockSurface(screen);
		SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0x28, 0x28, 0x28, 0xFF));
		SDL_GetWindowSize(window, &w, &h);
		SDL_RenderCopy(renderer, screentex, NULL, (w >= 4/3 * h) ? &(SDL_Rect){(w - (h * 4/3)) / 2, 0, h * 4/3, h} : &(SDL_Rect){0, (h - (w * 3/4)) / 2, w, w * 3/4});
		SDL_RenderPresent(renderer);

		if (extra.filereq >= 0) {
			free(assptrs.curfile);
			char fname[0b10'0000'0000] = "./Assets/";
			strcat(fname, assptrs.filenames[extra.filereq]);
			strcat(fname, ".json");
			struct stat fstatus = {0};
			stat(fname, &fstatus);
			FILE * file = fopen(fname, "r");
			assptrs.curfile = malloc(fstatus.st_size);
			assptrs.curflen = fstatus.st_size;
			fread(assptrs.curfile, fstatus.st_size, 1, file);
			fclose(file);
		} else {
			free(assptrs.curfile);  assptrs.curfile = NULL;
		}
		if (extra.quit)
			break;
		if (extra.swap) {
			inmenu =! inmenu;
			extra.swap = false;
		}

		timespec_get(&secondtime, TIME_UTC);
		secondtime.tv_nsec -= firsttime.tv_nsec; secondtime.tv_sec -= firsttime.tv_sec;
		if (secondtime.tv_sec > 0)
			continue;
		if (secondtime.tv_nsec > 27777777)
			continue;
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
