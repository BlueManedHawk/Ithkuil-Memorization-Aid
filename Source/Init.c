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
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include "../Libraries/json.h"

/* This function returns a structure of pointers to the various assets.  Since this is a structure of pointers, it's small enough that we can just return it directly.
 *
 * The font is loaded immediately, since it's necessary all the time, but the files aren't loaded until they're needed, so the structure just returns the filenames.  */

[[clang::optnone]] struct assptrs load_assptrs(void)
{
	DIR * dir = opendir("./Assets");
	if (dir == NULL) goto fail;

	struct assptrs assptrs = {0};
	assptrs.barlow_condensed = TTF_OpenFont("./Assets/BarlowCondensed-Regular.ttf", 12);
	if (assptrs.barlow_condensed == NULL) goto fail;

	assptrs.filenames = NULL;
	struct dirent * dirent = NULL;
	while ((dirent = readdir(dir)) != NULL) {
		/* TODO:  This is probably horrifically innefficient.  Is there any better way to do this? */
		char name[0b10'0000'0000] = "./Assets/";
		strcat(name, dirent->d_name);
		struct stat fstatus;
		stat(name, &fstatus);
		char * buf = malloc(fstatus.st_size);
		FILE * file = fopen(name, "r");
		fread(buf, fstatus.st_size, 1, file);
		fclose(file);
		json_value * tree;
		if ((tree = json_parse(buf, fstatus.st_size)) != NULL){
			json_value_free(tree);
			long i = assptrs.filetotal;
			assptrs.filenames = realloc(assptrs.filenames, sizeof (char *) * assptrs.filetotal + 1);
			assptrs.filenames[i] = calloc(strlen(dirent->d_name) + 1, sizeof (char));
			strcpy(assptrs.filenames[i], dirent->d_name);
			assptrs.filenames[i][strlen(assptrs.filenames[i]) - 5] = '\0';
			assptrs.filetotal++;
		}
		free(buf);
	}
	if (assptrs.filetotal == 0) goto fail;

	return assptrs;

fail:
	printf("\033[31mThe program's assets could not be loaded and the program will now exit.  Apologies for the inconvenience.\033[m\n");
	exit(EXIT_FAILURE);
}

void unload_assptrs(struct assptrs assptrs)
{
	TTF_CloseFont(assptrs.barlow_condensed);
	for (register long i = 0; i < assptrs.filetotal - 1 /*‽‽‽‽‽‽‽*/; i++)
		free(assptrs.filenames[i]);
}
