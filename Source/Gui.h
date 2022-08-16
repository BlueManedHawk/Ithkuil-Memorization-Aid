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

#ifndef GUI_H
#define GUI_H

#include "SDL.h"
#include <SDL2/SDL_ttf.h>
#include <stdbool.h> // TODO:  Once we can upgrade to Clang 15, get this out of here.

enum [[clang::enum_extensibility(closed)]] buttonstate {
	common = 0,
	hovered = 1,
	clicked = 2
};

struct buttondata {
	int pos[2];
	SDL_Surface * surfaces[3];
};

__extension__ extern SDL_Surface * draw_button(enum buttonstate, const char *, SDL_Rect extent, TTF_Font *, int, SDL_Color) __attribute__((diagnose_if(extent.x != 0 || extent.y != 0, "X and Y of rectangle must be zeros", "error")));
/* For whatever reason, the above attribute doesn't work if one uses the standard syntax, and _that_ doesn't work unless its declared as being an extension.  ???????
 *
 * Anyway, that should probably have a preprocessor check in place for it. */

[[gnu::malloc]] extern struct buttondata * alloc_button(const char *, SDL_Color, short, TTF_Font *);
extern void free_button(struct buttondata *);

/* extern void blit_apt_buttonstate(struct buttondata *, SDL_Surface *, SDL_Point, bool, bool, void (^)(void)); */

/* This macro is terrible, and it _really_ ought to be a function instead; however, i wasn't able to get the blocks runtime to link correctly on my system, so i decided to use this instead.  Note that this requires you to predefine an SDL_Rect called `dest` before it works—yet another reason why this is fucking garbage. */

#define BLIT_APT_BUTTONSTATE(MACRO_INTERNAL_button, MACRO_INTERNAL_blit_to, MACRO_INTERNAL_clickloc, MACRO_INTERNAL_isclicked, MACRO_INTERNAL_isreleased, MACRO_INTERNAL_pseudocallback)\
	dest.x = MACRO_INTERNAL_button->pos[0]; dest.y = MACRO_INTERNAL_button->pos[1];\
	dest.w = MACRO_INTERNAL_button->surfaces[0]->w; dest.h = MACRO_INTERNAL_button->surfaces[0]->h;\
	if (SDL_PointInRect(&MACRO_INTERNAL_clickloc, &dest)) {\
		if (MACRO_INTERNAL_isclicked) {\
			SDL_BlitSurface(MACRO_INTERNAL_button->surfaces[2], NULL, MACRO_INTERNAL_blit_to, &dest);\
		} else {\
			SDL_BlitSurface(MACRO_INTERNAL_button->surfaces[1], NULL, MACRO_INTERNAL_blit_to, &dest);\
			if (MACRO_INTERNAL_isreleased) MACRO_INTERNAL_pseudocallback;\
		}\
	} else {\
		SDL_BlitSurface(MACRO_INTERNAL_button->surfaces[0], NULL, MACRO_INTERNAL_blit_to, &dest);\
	}

#endif/*def GUI_H*/
