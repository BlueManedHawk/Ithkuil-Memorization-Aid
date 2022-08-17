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
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // TODO:  Once we can move to Clang 15, get this out of here.

static struct buttondata ** categories;
/* TODO:  The quit button and timer appear in both modes.  Ideally, they would be handled by the main function instead of each mode handling them separately using the same code. */
static struct buttondata * quitbutton, * timer[3];
static SDL_Surface * timertxt_surface;

static SDL_Surface * toptext;
static bool cached;

static const short padding = 12;

static void menu_cleanup(struct assptrs assptrs)
{
	for (register long i = 0; i < assptrs.filetotal; i++)
		free_button(categories[i]);
	free(categories);

	SDL_FreeSurface(toptext);
	SDL_FreeSurface(timertxt_surface);
	free_button(quitbutton);

	for (register long i = 0; i < 3; i++)
		free_button(timer[i]);
}

void menu_render(SDL_Surface * screen, struct assptrs assptrs)
{
	SDL_Event e;
	SDL_Point clickloc = {0};
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
	/* TODO:  scaling.  This will also require work in `Source/Main.c` and `Source/Questions.c`. */
	SDL_PumpEvents();
	click = (SDL_GetMouseState(&clickloc.x, &clickloc.y) & SDL_BUTTON_LMASK) ? true : false;

	if (!cached) [[clang::unlikely]] {
		/* TODO: it's a bit confusing how this actually works.  A rewrite may be in order. */
		for (register long i = 0; i < assptrs.filetotal; i++){
			categories = realloc(categories, (sizeof (struct buttondata) * (i+1)));
			categories[i] = alloc_button(assptrs.filenames[i], (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		}

		int breaks[][2] = {0}; // [0] is used for location in assptrs.filetotal, whereas [1] is used for the location in pixels…i think.
		int queue = 0;
		int lines = 0;
		for (register long i = 0; i < assptrs.filetotal; i++) {
			queue += categories[i]->surfaces[0]->w + padding;
			if (queue > screenwidth) {
				queue = 0;
				breaks[lines][0] = i;
				breaks[lines][1] -= padding;
				lines++;
			} else {
				breaks[lines][1] += categories[i]->surfaces[0]->w + padding;
			}
		}
		breaks[lines][0] = -1; breaks[lines][1] /= 2;  // I have only a vague idea of what this line does.  Removing it makes the buttons not center correctly.

		queue = 0;
		lines = 0;
		for (register long i = 0; i < assptrs.filetotal; i++) {
			categories[i]->pos[0] = screenwidth / 2 - breaks[lines][1] + queue;
			categories[i]->pos[1] = (lines * (categories[i]->surfaces[0]->h)) + 48;
			if (breaks[lines][0] == i) {
				queue = 0;
				lines++;
			} else {
				queue += categories[i]->surfaces[0]->w + padding;
			}
		}

		quitbutton = alloc_button("Quit", (SDL_Color){0xff, 0x44, 0x44, 0xff}, 12, assptrs.barlow_condensed);
		quitbutton->pos[0] = 6; quitbutton->pos[1] = screenheight - 6 - quitbutton->surfaces[0]->h;

		timer[0] = alloc_button("↑", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timer[0]->pos[0] = screenwidth - 6 - timer[0]->surfaces[0]->w; timer[0]->pos[1] = screenheight - 6 - (timer[0]->surfaces[0]->h * 2);
		timer[1] = alloc_button("↓", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timer[1]->pos[0] = screenwidth - 6 - timer[1]->surfaces[0]->w; timer[1]->pos[1] = screenheight - 6 - timer[1]->surfaces[0]->h;
		timer[2] = alloc_button("↻", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timer[2]->pos[0] = screenwidth - 12 - timer[2]->surfaces[0]->w * 2; timer[2]->pos[1] = screenheight - 6 - (timer[2]->surfaces[0]->h * 2);

		timertxt_surface = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, "No timer", (SDL_Color){0xff, 0xff, 0xff, 0xff});

		TTF_SetFontSize(assptrs.barlow_condensed, 24);
		TTF_SetFontWrappedAlign(assptrs.barlow_condensed, TTF_WRAPPED_ALIGN_CENTER);
		toptext = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, "Welcome to ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls!  Please select a category.", (SDL_Color){0xff, 0xff, 0xff, 0xff}, screenwidth);

		cached = true;
	}

	SDL_Rect dest = {0};
	for (register long i = 0; i < assptrs.filetotal; i++) {
		BLIT_APT_BUTTONSTATE(categories[i], screen, clickloc, click, release, {
				((struct extra *)screen->userdata)->swap = true;
				((struct extra *)screen->userdata)->filereq = i;
			});
	}

	BLIT_APT_BUTTONSTATE(quitbutton, screen, clickloc, click, release, {SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});});  // could just directly set the quit state, but this is sillier and more fun

	BLIT_APT_BUTTONSTATE(timer[2], screen, clickloc, click, release, {((struct extra *)screen->userdata)->timer = 0;});
	BLIT_APT_BUTTONSTATE(timer[1], screen, clickloc, click, release, {if (((struct extra *)screen->userdata)->timer != 0) ((struct extra *)screen->userdata)->timer--;});
	BLIT_APT_BUTTONSTATE(timer[0], screen, clickloc, click, release, {((struct extra *)screen->userdata)->timer++;});

	char timertxt[7 + 6 + 1]; // space for words + 16-bit largest integer length as string + NUL
	if (((struct extra *)(screen->userdata))->timer != 0)
		sprintf(timertxt, "Timer: %d", ((struct extra *)screen->userdata)->timer);
	else
		sprintf(timertxt, "No timer");
	SDL_FreeSurface(timertxt_surface);
	TTF_SetFontSize(assptrs.barlow_condensed, 12);
	timertxt_surface = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, timertxt, (SDL_Color){0xff, 0xff, 0xff, 0xff});
	dest.y = timer[1]->pos[1]; dest.x = timer[1]->pos[0] - timertxt_surface->w - 6;
	dest.w = timertxt_surface->w; dest.h = timertxt_surface->h;
	SDL_BlitSurface(timertxt_surface, NULL, screen, &dest);

	dest.x = (screenwidth - toptext->w) / 2; dest.y = 0;
	SDL_BlitSurface(toptext, NULL, screen, &dest);

	const SDL_Rect mptr = {.x = clickloc.x, .y = clickloc.y, .w = 1, .h = 1};
	SDL_FillRect(screen, &mptr, SDL_MapRGBA(screen->format, 0xFF, 0x00, 0x00, 0xFF));

	if (((struct extra *)screen->userdata)->quit) menu_cleanup(assptrs);
}
