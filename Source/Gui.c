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

/* This file contains some functions for the GUI used in both the questions and menu sections of the program. */

#include "SDL.h"
#include <SDL2/SDL_ttf.h>
#include <stddef.h>
#include "Gui.h"
#include <stdint.h>
#include <stdbool.h>  // TODO:  Once we can upgrade to Clang 15, get this out of here.

SDL_Surface * draw_button_with_text(enum buttonstate state, const char * txt, SDL_Rect extent, TTF_Font * font, int fontsize, SDL_Color color)
{
	[[maybe_unused]] static const short width_pad = 6;
	static const short button_depth = 6;

	SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormat(0, extent.w, extent.h + button_depth, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Rect side = extent; side.y += button_depth;
	uint32_t sidecolor, topcolor;

	switch (state){
	case common:
		sidecolor = SDL_MapRGBA(surface->format, 0x44, 0x44, 0x44, 0xFF);
		topcolor = SDL_MapRGBA(surface->format, 0x66, 0x66, 0x66, 0xFF);
		break;
	case hovered:
		sidecolor = SDL_MapRGBA(surface->format, 0x66, 0x66, 0x66, 0xFF);
		topcolor = SDL_MapRGBA(surface->format, 0x88, 0x88, 0x88, 0xFF);
		break;
	case clicked:
		sidecolor = SDL_MapRGBA(surface->format, 0x88, 0x88, 0x88, 0xFF);
		break;
	}

	TTF_SetFontSize(font, fontsize);
	TTF_SetFontWrappedAlign(font, TTF_WRAPPED_ALIGN_CENTER);
	SDL_Surface * txt_surface = TTF_RenderUTF8_Blended_Wrapped(font, txt, color, extent.w);

	if (state == clicked){
		SDL_FillRect(surface, &side, sidecolor);
		SDL_BlitSurface(txt_surface, NULL, surface, &side);
	} else {
		SDL_FillRect(surface, &side, sidecolor);
		SDL_FillRect(surface, &extent, topcolor);
		SDL_BlitSurface(txt_surface, NULL, surface, &(SDL_Rect){.x = 000000000000000000000000000000000000 , .y = 0});
	}

	SDL_FreeSurface(txt_surface);
	return surface;
}

struct buttondata * alloc_button(const char * text, SDL_Color color, short ptsize, TTF_Font * font)
{
	int h, w;
	TTF_SetFontSize(font, ptsize);
	TTF_SizeUTF8(font, text, &w, &h);
	struct buttondata * button = malloc(sizeof (struct buttondata));
	for (register short i = 0; i < 3; i++)
		button->surfaces[i] = draw_button_with_text((enum buttonstate) i, text, (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, font, ptsize, color);
	return button;
}

void free_button(struct buttondata * button)
{
	for (register size_t i = 0; i < sizeof button->surfaces / sizeof button->surfaces[0]; i++)
		SDL_FreeSurface(button->surfaces[i]);
	free(button);
}

/* void blit_apt_buttonstate(struct buttondata * button, SDL_Surface * blit_to, SDL_Point clickloc, bool isclicked, bool isreleased, void (^release_callback)(void)) {
	SDL_Rect dest = {0};
	dest.x = button->pos[0]; dest.y = button->pos[1];
	dest.w = button->surfaces[0]->w; dest.h = button->surfaces[0]->h;
	if (SDL_PointInRect(&clickloc, &dest)) {
		if (isclicked) {
			SDL_BlitSurface(button->surfaces[2], NULL, blit_to, &dest);
		} else {
			SDL_BlitSurface(button->surfaces[1], NULL, blit_to, &dest);
			if (isreleased)
				release_callback();
		}
	} else {
		SDL_BlitSurface(button->surfaces[0], NULL, blit_to, &dest);
	}
} */
