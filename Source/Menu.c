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
#include <stdbool.h> // TODO:  Once we can move to Clang 15, get this out of here.

/* Theoretically, we could use the `.userdata` field of SDL_surfaces instead of using an array of structs, but that would require like linked lists and more complicated code and we'd probably end up using structs somewhere in it anyway so we just go with this.  There's also the worry that doing things with `.userdata` could make it seem like we care about elegance, which we don't.  We care about practicality—that's why this is written in C, after all! */
struct buttondata {int x; int y; SDL_Surface * surfaces[3];};
static struct buttondata * buttons = NULL;

static SDL_Surface * toptext;
static bool cached = false;

static const short padding = 12;


static void menu_cleanup(struct assptrs assptrs)
{
	for (register long i = 0; i < assptrs.filetotal; i++){
		for (register short j = 0; j < 3; j++)
			SDL_FreeSurface(buttons[i].surfaces[j]);
	}
	SDL_FreeSurface(toptext);
	free(buttons);
}

void menu_render(SDL_Surface * screen, struct assptrs assptrs)
{
	SDL_Event e;
	int mousepos[2] = {0};
	bool click = false;
	while (SDL_PollEvent(&e)){
		switch (e.type){
		case SDL_QUIT:
			((struct extra *)screen->userdata)->quit = true;
			break;
		}
	}
	SDL_PumpEvents();
	click = (SDL_GetMouseState(&mousepos[0], &mousepos[1]) & SDL_BUTTON_LMASK) ? true : false;

	if (!__builtin_expect_with_probability(cached, 1, 1.0)) [[clang::unlikely]] {  // Using both `__builtin_expect_with_probability()` _and_ `[[clang::unlikely]]` is probably a bit overkill.
		/* TODO: this is hard to mentally parse.  I ought to rewrite it sometime. */
		TTF_SetFontSize(assptrs.barlow_condensed, 12);
		int h, w;
		for (register long i = 0; i < assptrs.filetotal; i++){
			buttons = realloc(buttons, (sizeof (struct buttondata) * (i+1)));
			TTF_SizeUTF8(assptrs.barlow_condensed, assptrs.filenames[i], &w, &h);
			for (register short j = 0; j < 3; j++)
				buttons[i].surfaces[j] = draw_button_with_text((enum buttonstate) j, assptrs.filenames[i], (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, assptrs.barlow_condensed, 12, (SDL_Color){0xff, 0xff, 0xff, 0xff});
		}

		int breaks[][2] = {0};
		int queue = 0;
		short a = 0;
		for (register long i = 0; i < assptrs.filetotal; i++) {
			queue += buttons[i].surfaces[0]->w + padding;
			if (queue > screenwidth) {
				queue = 0;
				breaks[a][0] = i;
				breaks[a][1] -= padding;
				a++;
			} else {
				breaks[a][1] += buttons[i].surfaces[0]->w + padding;
			}
		}
		breaks[a][0] = -1; breaks[a][1] /= 2;  // I have no idea what this line does, but removing it makes the buttons not center correctly.

		queue = 0; // stupid variable reuse—this ain't the gameboy
		a = 0;
		for (register long i = 0; i < assptrs.filetotal; i++) {
			buttons[i].x = screenwidth / 2 - breaks[a][1] + queue;
			buttons[i].y = (a * (buttons[i].surfaces[0]->h)) + 48;
			if (breaks[a][0] == i) {
				queue = 0;
				a++;
			} else {
				queue += buttons[i].surfaces[0]->w + padding;
			}
		}

		TTF_SetFontSize(assptrs.barlow_condensed, 24);
		toptext = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, "Welcome to ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls!  Please select a category.", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 640);
		cached = true;
	}

	SDL_Rect dest = {0};
	SDL_Point clickloc = {.x = mousepos[0], .y = mousepos[1]};
	for (register long i = 0; i < assptrs.filetotal; i++){
		dest.x = buttons[i].x; dest.y = buttons[i].y;
		dest.w = buttons[i].surfaces[0]->w; dest.h = buttons[i].surfaces[0]->h;
		if (SDL_PointInRect(&clickloc, &dest)){
			if (click) SDL_BlitSurface(buttons[i].surfaces[2], NULL, screen, &dest);
			else SDL_BlitSurface(buttons[i].surfaces[1], NULL, screen, &dest);
		} else {
			SDL_BlitSurface(buttons[i].surfaces[0], NULL, screen, &dest);
		}
	}
	dest.x = (screenwidth - toptext->w) / 2; dest.y = 0;
	SDL_BlitSurface(toptext, NULL, screen, &dest);
	const SDL_Rect mptr = {.x = clickloc.x, .y = clickloc.y, .w = 1, .h = 1};
	SDL_FillRect(screen, &mptr, SDL_MapRGBA(screen->format, 0xFF, 0x00, 0x00, 0xFF));
	if (((struct extra *)screen->userdata)->quit) menu_cleanup(assptrs);
}
