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

#ifndef INIT_H
#define INIT_H

#include "SDL2/SDL_ttf.h"
#include <stdio.h>
#include <stdbool.h> // TODO:  Once we update to Clang 15, get this out of here.

static const int screenwidth = 640, screenheight = 480;

struct assptrs {
	TTF_Font * barlow_condensed;
	FILE * curfile;
	long filetotal;
	/* I just got done fixing a bizarre bug with this that occupied me for far too long, so i'm going to take this space to rant.
	 *
	 * This was initially `char * filenames[]`, which would have made things a lot easier to deal with, except that that kept leading to segfaults, because after the assptrs were loaded, this array would inevitably end up getting overwritten by _something_ (usually a function to which this wasn't even passed!) before it got passed on to the rendering subroutines.  Weirdly, this wouldn't happen if optimization was turned off, but of course we can't assume that everybody can afford that.  So, eventually, i just made the decision to have this be a dynamically allocated pointer-pointer, and that ended up working at the cost of being fuck-ass ugly and probably also kinda innefficient.
	 *
	 * So that was fun.
	 *
	 * — Blue-Maned_Hawk */
	char ** filenames;
};

struct extra {
	bool quit;
	bool swap;
	int16_t timer;
	signed short filereq;
};

extern struct assptrs load_assptrs(void);
extern void unload_assptrs(struct assptrs);

#endif
