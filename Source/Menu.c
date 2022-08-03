/* LICENSE
 *
 * Copyright © 2022 Blue-Maned_Hawk.  All rights reserved.
 *
 * This project should have come with a file called `LICENSE`.  In the event of any conflict between this comment and that file, that file shall be considered the authority.
 *
 * You may freely use this software.  You may freely distribute this software, so long as you distribute the license and source code with it.  You may freely modify this software and distribute the modifications under a similar license, so long as you distribute the sources with them and you don't claim that they're the original software.  None of this overrides local laws, and if you excercise these rights, you cannot claim that your actions are condoned by the author.
 *
 * This license does not apply to patents or trademarks.
 *
 * This software comes with no warranty, implied or explicit.  The author disclaims any liability for damages caused by this software. */

/* This file handles the main menu for ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls. */

#include "SDL.h"
#include "Menu.h"
#include "Init.h"
#include "Gui.h"
#include "SDL2/SDL_ttf.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // TODO:  Once we can move to Clang 15, get this out of here.

/* Theoretically, we could use the `.userdata` field of SDL_surfaces instead of using an array of structs, but that would require like linked lists and more complicated code and we'd probably end up using structs somewhere in it anyway so we just go with this.  There's also the worry that doing things with `.userdata` could make it seem like we care about elegance, which we don't.  We care about practicality—that's why this is written in C, after all! */
struct buttondata {int x; int y; SDL_Surface * surfaces[3];};
static struct buttondata * categories;
static struct buttondata quitbutton, timer[3];
static SDL_Surface * timertxt_surface;

static SDL_Surface * toptext;
static bool cached;

static const short padding = 12;

static void menu_cleanup(struct assptrs assptrs)
{
	for (register long i = 0; i < assptrs.filetotal; i++) {
		for (register short j = 0; j < 3; j++)
			SDL_FreeSurface(categories[i].surfaces[j]);
	}
	free(categories);

	SDL_FreeSurface(toptext);
	SDL_FreeSurface(timertxt_surface);
	for (register short j = 0; j < 3; j++)
		SDL_FreeSurface(quitbutton.surfaces[j]);

	for (register long i = 0; i < 3; i++) {
		for (register short j = 0; j < 3; j++)
			SDL_FreeSurface(timer[i].surfaces[j]);
	}

}

void menu_render(SDL_Surface * screen, struct assptrs assptrs)
{
	SDL_Event e;
	int mousepos[2] = {0};
	bool click = false;
	bool release = false;
	while (SDL_PollEvent(&e)){
		switch (e.type){
		case SDL_QUIT:
			((struct extra *)screen->userdata)->quit = true;
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.button == SDL_BUTTON_LEFT)
				release = true;
		}
	}
	SDL_PumpEvents();
	click = (SDL_GetMouseState(&mousepos[0], &mousepos[1]) & SDL_BUTTON_LMASK) ? true : false;

	if (!__builtin_expect_with_probability(cached, true, 1.0)) [[clang::unlikely]] {  // Using both `__builtin_expect_with_probability()` _and_ `[[clang::unlikely]]` is probably a bit overkill.
		/* TODO: this is hard to mentally parse.  I ought to rewrite it sometime. */
		TTF_SetFontSize(assptrs.barlow_condensed, 12);
		int h, w;
		for (register long i = 0; i < assptrs.filetotal; i++){
			categories = realloc(categories, (sizeof (struct buttondata) * (i+1)));
			TTF_SizeUTF8(assptrs.barlow_condensed, assptrs.filenames[i], &w, &h);
			for (register short j = 0; j < 3; j++)
				categories[i].surfaces[j] = draw_button_with_text((enum buttonstate) j, assptrs.filenames[i], (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, assptrs.barlow_condensed, 12, (SDL_Color){0xff, 0xff, 0xff, 0xff});
		}

		int breaks[][2] = {0};
		int queue = 0;
		short a = 0;
		for (register long i = 0; i < assptrs.filetotal; i++) {
			queue += categories[i].surfaces[0]->w + padding;
			if (queue > screenwidth) {
				queue = 0;
				breaks[a][0] = i;
				breaks[a][1] -= padding;
				a++;
			} else {
				breaks[a][1] += categories[i].surfaces[0]->w + padding;
			}
		}
		breaks[a][0] = -1; breaks[a][1] /= 2;  // I have no idea what this line does, but removing it makes the buttons not center correctly.

		queue = 0; // stupid variable reuse—this ain't the gameboy
		a = 0;
		for (register long i = 0; i < assptrs.filetotal; i++) {
			categories[i].x = screenwidth / 2 - breaks[a][1] + queue;
			categories[i].y = (a * (categories[i].surfaces[0]->h)) + 48;
			if (breaks[a][0] == i) {
				queue = 0;
				a++;
			} else {
				queue += categories[i].surfaces[0]->w + padding;
			}
		}

		TTF_SizeUTF8(assptrs.barlow_condensed, "Quit", &w, &h);
		for (register short i = 0; i < 3; i++)
			quitbutton.surfaces[i] = draw_button_with_text((enum buttonstate) i, "Quit", (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, assptrs.barlow_condensed, 12, (SDL_Color){0xff, 0x44, 0x44, 0xff});
		quitbutton.x = 6; quitbutton.y = screenheight - 12 - h;

		TTF_SizeUTF8(assptrs.barlow_condensed, "↑", &w, &h);
		for (register short i = 0; i < 3; i++)
			timer[0].surfaces[i] = draw_button_with_text((enum buttonstate) i, "↑", (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, assptrs.barlow_condensed, 12, (SDL_Color){0xff, 0xff, 0xff, 0xff});
		timer[0].x = screenwidth - 12 - w; timer[0].y = screenheight - 12 - (h * 2 + 6);

		TTF_SizeUTF8(assptrs.barlow_condensed, "↓", &w, &h);
		for (register short i = 0; i < 3; i++)
			timer[1].surfaces[i] = draw_button_with_text((enum buttonstate) i, "↓", (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, assptrs.barlow_condensed, 12, (SDL_Color){0xff, 0xff, 0xff, 0xff});
		timer[1].x = screenwidth - 12 - w; timer[1].y = screenheight - 12 - h;

		TTF_SizeUTF8(assptrs.barlow_condensed, "↻", &w, &h);
		for (register short i = 0; i < 3; i++)
			timer[2].surfaces[i] = draw_button_with_text((enum buttonstate) i, "↻", (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, assptrs.barlow_condensed, 12, (SDL_Color){0xff, 0xff, 0xff, 0xff});
		timer[2].x = screenwidth - 24 - w * 2; timer[2].y = screenheight - 12 - (h * 2 + 6);

		timertxt_surface = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, "No timer", (SDL_Color){0xff, 0xff, 0xff, 0xff});

		TTF_SetFontSize(assptrs.barlow_condensed, 24);
		toptext = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, "Welcome to ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls!  Please select a category.", (SDL_Color){0xff, 0xff, 0xff, 0xff}, screenwidth);
		cached = true;
	}

	SDL_Rect dest = {0};
	SDL_Point clickloc = {.x = mousepos[0], .y = mousepos[1]};
	for (register long i = 0; i < assptrs.filetotal; i++) {
		dest.x = categories[i].x; dest.y = categories[i].y;
		dest.w = categories[i].surfaces[0]->w; dest.h = categories[i].surfaces[0]->h;
		if (SDL_PointInRect(&clickloc, &dest)) {
			if (click) {
				SDL_BlitSurface(categories[i].surfaces[2], NULL, screen, &dest);
			} else {
				SDL_BlitSurface(categories[i].surfaces[1], NULL, screen, &dest);
				if (release){
					((struct extra *)(screen->userdata))->swap = true;
					((struct extra *)(screen->userdata))->filereq = i;
				}
			}
		} else {
			SDL_BlitSurface(categories[i].surfaces[0], NULL, screen, &dest);
		}
	}

	dest.x = quitbutton.x; dest.y = quitbutton.y;
	dest.w = quitbutton.surfaces[1]->w; dest.h = quitbutton.surfaces[0]->h;
	if (SDL_PointInRect(&clickloc, &dest)) {
		if (click) SDL_BlitSurface(quitbutton.surfaces[2], NULL, screen, &dest);
		else if (release) SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});  // could just directly set the quit state, but this is sillier and more fun
		else SDL_BlitSurface(quitbutton.surfaces[1], NULL, screen, &dest);
	} else {
		SDL_BlitSurface(quitbutton.surfaces[0], NULL, screen, &dest);
	}

	for (register short i = 0; i < 3; i++) {
		dest.x = timer[i].x; dest.y = timer[i].y;
		dest.w = timer[i].surfaces[0]->w; dest.h = timer[i].surfaces[0]->h;
		if (SDL_PointInRect(&clickloc, &dest)) {
			if (click) {
				SDL_BlitSurface(timer[i].surfaces[2], NULL, screen, &dest);
			} else {
				if (release) switch (i){
				case 2:
					((struct extra *)(screen->userdata))->timer = 0;
					break;
				case 1:
					if (((struct extra *)(screen->userdata))->timer != 0)
						((struct extra *)(screen->userdata))->timer--;
					break;
				case 0:
					((struct extra *)(screen->userdata))->timer++;
					break;
				}
				SDL_BlitSurface(timer[i].surfaces[1], NULL, screen, &dest);
			}
		} else {
			SDL_BlitSurface(timer[i].surfaces[0], NULL, screen, &dest);
		}
	}

	char timertxt[7 + 6 + 1]; // space for words + 16-bit largest integer length as string + NUL
	if (((struct extra *)(screen->userdata))->timer != 0)
		sprintf(timertxt, "Timer: %d", ((struct extra *)(screen->userdata))->timer);
	else
		sprintf(timertxt, "No timer");
	SDL_FreeSurface(timertxt_surface);
	TTF_SetFontSize(assptrs.barlow_condensed, 12);
	timertxt_surface = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, timertxt, (SDL_Color){0xff, 0xff, 0xff, 0xff});
	dest.y = timer[1].y; dest.x = timer[1].x - timertxt_surface->w - 6;
	dest.w = timertxt_surface->w; dest.h = timertxt_surface->h;
	SDL_BlitSurface(timertxt_surface, NULL, screen, &dest);
	dest.x = (screenwidth - toptext->w) / 2; dest.y = 0;
	SDL_BlitSurface(toptext, NULL, screen, &dest);
	const SDL_Rect mptr = {.x = clickloc.x, .y = clickloc.y, .w = 1, .h = 1};
	SDL_FillRect(screen, &mptr, SDL_MapRGBA(screen->format, 0xFF, 0x00, 0x00, 0xFF));
	if (((struct extra *)screen->userdata)->quit) menu_cleanup(assptrs);
}
