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

#ifndef GUI_H
#define GUI_H

#include "SDL.h"
#include <SDL2/SDL_ttf.h>

enum [[clang::enum_extensibility(closed)]] buttonstate {
	common = 0,
	hovered = 1,
	clicked = 2
};

struct buttondata {
	int pos[2];
	SDL_Surface * surfaces[3];
};

/* For whatever reason, the  attribute doesn't work if one uses the standard syntax, and _that_ doesn't work unless its declared as being an extension.  ??????? */
__extension__ extern SDL_Surface * draw_button(enum buttonstate, const char *, SDL_Rect extent, TTF_Font *, int, SDL_Color) __attribute__((diagnose_if(extent.x != 0 || extent.y != 0, "X and Y of rectangle must be zeros", "error")));

[[gnu::malloc]] /* Note that Clang does not support GCC's version of this attribute with arguments. */ extern struct buttondata * alloc_button(const char *, SDL_Color, short, TTF_Font *);
extern void free_button(struct buttondata *);

/* extern void blit_apt_buttonstate(struct buttondata *, SDL_Surface *, SDL_Point, bool, bool, void (^)(void)); */

#define BLIT_APT_BUTTONSTATE(button, blit_to, clickloc, isclicked, isreleased, pseudocallback) do {\
	SDL_Rect dest;\
	dest.x = button->pos[0]; dest.y = button->pos[1];\
	dest.w = button->surfaces[0]->w; dest.h = button->surfaces[0]->h;\
	if (SDL_PointInRect(&clickloc, &dest)) {\
		if (isclicked) {\
			SDL_BlitSurface(button->surfaces[2], NULL, blit_to, &dest);\
		} else {\
			SDL_BlitSurface(button->surfaces[1], NULL, blit_to, &dest);\
			if (isreleased) pseudocallback;\
		}\
	} else {\
		SDL_BlitSurface(button->surfaces[0], NULL, blit_to, &dest);\
	}\
} while (false);

#endif/*def GUI_H*/
