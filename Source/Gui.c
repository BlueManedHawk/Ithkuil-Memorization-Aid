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

/* This file contains some functions for the GUI used in both the questions and menu sections of the program. */

#include "SDL.h"
#include <SDL2/SDL_ttf.h>
#include <stddef.h>
#include "Gui.h"
#include <stdint.h>

SDL_Surface * draw_button_with_text(enum buttonstate state, const char * txt, SDL_Rect extent, TTF_Font * font, int fontsize, SDL_Color color)
{
	[[maybe_unused]] static const short width_pad = 6;
	static const short button_depth = 6;

	SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormat(0, extent.w, extent.h + button_depth, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Rect side = extent; side.y += button_depth;
	uint32_t sidecolor, topcolor;

	switch (state){
	case common:
		sidecolor = SDL_MapRGBA(surface->format, 0x3C, 0x38, 0x36, 0xFF);
		topcolor = SDL_MapRGBA(surface->format, 0x50, 0x49, 0x45, 0xFF);
		break;
	case hovered:
		sidecolor = SDL_MapRGBA(surface->format, 0x50, 0x49, 0x45, 0xFF);
		topcolor = SDL_MapRGBA(surface->format, 0x66, 0x5C, 0x54, 0xFF);
		break;
	case clicked:
		sidecolor = SDL_MapRGBA(surface->format, 0x66, 0x5C, 0x54, 0xFF);
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
