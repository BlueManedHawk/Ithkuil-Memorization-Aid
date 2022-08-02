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

/* TODO: Once SDL_ttf 2.20 releases, we should make buttons render with the text's wrapping alignment centered. */
SDL_Surface * draw_button_with_text(enum buttonstate state, const char * txt, SDL_Rect extent, TTF_Font * font, int fontsize, SDL_Color color)
{
	static const short w_pad = 6;
	static const short button_travel = 6;

	SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormat(0, extent.w, extent.h + button_travel, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Rect side = extent; side.y += button_travel;
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
	SDL_Surface * txt_surface = TTF_RenderUTF8_Blended_Wrapped(font, txt, color, extent.w + w_pad);

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
