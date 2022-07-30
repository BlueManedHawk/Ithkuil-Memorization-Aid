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

struct buttondata {int x; int y; SDL_Surface * surfaces[3];};

static struct buttondata * buttons = NULL;
static SDL_Surface * toptext;
static bool cached = false;

static const short padding = 12;

struct menu_info menu_handle_events(void)
{
	struct menu_info m = {0};
	SDL_Event e;
	while (SDL_PollEvent(&e)){
		switch (e.type){
		case SDL_QUIT:
			m.quit = true;
			break;
		}
	}
	SDL_PumpEvents();
	if (SDL_GetMouseState(&m.pos[0], &m.pos[1]) & SDL_BUTTON_LMASK) m.click = true;
	return m;
}

/* TODO: this is hard to mentally parse.  I ought to rewrite it sometime. */
void menu_render(SDL_Renderer * renderer, struct menu_info menu_info, struct assptrs assptrs)
{
	if (!__builtin_expect(cached, 1)) [[clang::unlikely]] {  // Using both `__builtin_expect()` _and_ `[[clang::unlikely]]` is probably a bit overkill.
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
		for (register long i = 0; i < assptrs.filetotal; i++){
			queue += buttons[i].surfaces[0]->w + padding;
			if (queue > 640){
				queue = 0;
				breaks[a][0] = i;
				breaks[a][1] -= padding; //ooo nice branchless
				a++;
			} else {
				breaks[a][1] += buttons[i].surfaces[0]->w + padding;
			}
		}
		breaks[a][0] = -1; breaks[a][1] /= 2;  // I have no idea what this line does, but removing it makes the buttons not center correctly.

		queue = 0; // stupid variable reuse—this ain't the gameboy
		a = 0;
		for (register long i = 0; i < assptrs.filetotal; i++){
			buttons[i].x = 640 / 2  - breaks[a][1] + queue;
			buttons[i].y = (a * (buttons[i].surfaces[0]->h)) + 48;
			if (breaks[a][0] == i){
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

	SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Rect dest = {0};
	const SDL_Point point = {.x = menu_info.pos[0], .y = menu_info.pos[1]};
	for (register long i = 0; i < assptrs.filetotal; i++){
		dest.x = buttons[i].x; dest.y = buttons[i].y;
		dest.w = buttons[i].surfaces[0]->w; dest.h = buttons[i].surfaces[0]->h;
		if (SDL_PointInRect(&point, &dest)){
			if (menu_info.click) SDL_BlitSurface(buttons[i].surfaces[2], NULL, surface, &dest);
			else SDL_BlitSurface(buttons[i].surfaces[1], NULL, surface, &dest);
		} else {
			SDL_BlitSurface(buttons[i].surfaces[0], NULL, surface, &dest);
		}
	}
	dest.x = (640 - toptext->w) / 2; dest.y = 0;
	SDL_BlitSurface(toptext, NULL, surface, &dest);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

void menu_cleanup(struct assptrs assptrs)
{
	for (register long i = 0; i < assptrs.filetotal; i++){
		for (register short j = 0; j < 3; j++)
			SDL_FreeSurface(buttons[i].surfaces[j]);
	}
	SDL_FreeSurface(toptext);
	free(buttons);
}
