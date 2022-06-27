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

/* TODO: Once SDL_ttf 2.20 releases, we should make buttons render with the text's wrapping alignment centered. */
SDL_Surface * draw_button_with_text(enum buttonstate state, const char * txt, SDL_Rect extent, TTF_Font * font, SDL_Color color){
	SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormat(0, extent.w, extent.h + 12, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Rect top = extent; top.y -= 12;
	uint32_t sidecolor, topcolor;

	switch (state){
	case common:
		sidecolor = SDL_MapRGBA(surface->format, 0x44, 0x44, 0x44, 0xFF);
		topcolor = SDL_MapRGBA(surface->format, 0x88, 0x88, 0x88, 0xFF);
		break;
	case hovered:
	case clicked:
		sidecolor = SDL_MapRGBA(surface->format, 0x88, 0x88, 0x88, 0xFF);
		topcolor = SDL_MapRGBA(surface->format, 0xCC, 0xCC, 0xCC, 0xFF);
		break;
	default:
		return NULL;
	}

	if (state == clicked){
		SDL_FillRect(surface, &extent, topcolor);
	} else {
		SDL_FillRect(surface, &extent, sidecolor);
		SDL_FillRect(surface, &top, topcolor);
	}

	SDL_Surface * txt_surface = TTF_RenderUTF8_Blended_Wrapped(font, txt, color, extent.w);

	SDL_BlitSurface(txt_surface, NULL, surface, &(SDL_Rect){.x = (extent.w - surface->w) / 2, .y = 0});
	SDL_FreeSurface(txt_surface);
	return surface;
}
