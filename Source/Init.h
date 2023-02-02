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

#ifndef INIT_H
#define INIT_H

#include "SDL2/SDL_ttf.h"
#include <stdio.h>
#include "../Libraries/json.h"

static const int screenwidth = 640, screenheight = 480;

struct assptrs {
	TTF_Font * barlow_condensed;
	json_value * curfile;
	long filetotal;
	/* dumbass didn't know what a vla member was 💀 */
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
