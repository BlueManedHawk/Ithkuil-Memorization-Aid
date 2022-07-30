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
#include "SDL2/SDL.h"

enum [[clang::enum_extensibility(closed)]] buttonstate {
	common = 0,
	hovered = 1,
	clicked = 2
};

__extension__ extern SDL_Surface * draw_button_with_text(enum buttonstate, const char *, SDL_Rect extent, TTF_Font *, int, SDL_Color) __attribute__((diagnose_if(extent.x != 0 || extent.y != 0, "X and Y of rectangle must be zeros", "error")));  // For whatever reason, this attribute doesn't work if one uses the standard syntax, and _that_ doesn't work unless its declared as being an extension.  ???????

#endif/*def GUI_H*/
