/* LICENSE
 * Copyright © 2022 Blue-Maned_Hawk.  All rights reserved.
 *
 * This project should have come with a file called `LICENSE`.  In the event of any conflict between this comment and that file, that file shall be considered the authority.
 *
 * You may freely use this software.  You may freely distribute this software, so long as you distribute the license and source code with it.  You may freely modify this software and distribute the modifications under a similar license, so long as you distribute the sources with them and you don't claim that they're the original software.  None of this overrides local laws, and if you excercise these rights, you cannot claim that your actions are condoned by the author.
 *
 * This license does not apply to patents or trademarks.
 *
 * This software comes with no warranty, implied or explicit.  The author disclaims any liability for damages caused by this software. */

/* This file contains subroutines used in the initialization and deinitialization of the software. */

#include "Init.h"
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

struct assets load_assets(void)
{
	DIR * dir = opendir("./Assets");
	if (dir == NULL) goto fail;

	struct assets assets = {0};
	assets.barlow_condensed = TTF_OpenFont("./Assets/BarlowCondensed-Regular.ttf", 12);
	if (assets.barlow_condensed == NULL) goto fail;

	return assets;

fail:
	printf("\033[31mThe program's assets could not be loaded and the program will now exit.  Apologies for the inconvenience.\033[0m\n");
	exit(EXIT_FAILURE);

}

void unload_assets(struct assets assets)
{
	TTF_CloseFont(assets.barlow_condensed);
}
