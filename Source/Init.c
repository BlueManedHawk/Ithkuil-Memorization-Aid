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
 * The font is loaded immediately, since it's necessary all the time, but the files aren't loaded until they're needed, so the structure just returns the filenames. */

struct assptrs load_assptrs(void)
{
	DIR * dir = opendir("./Assets");
	if (dir == NULL)
		goto fail;

	struct assptrs assptrs = {0};
	assptrs.barlow_condensed = TTF_OpenFont("./Assets/BarlowCondensed-Regular.ttf", 12);
	if (assptrs.barlow_condensed == NULL)
		goto fail;

	assptrs.filenames = NULL;
	struct dirent * dirent = NULL;
	while ((dirent = readdir(dir)) != NULL) {
		char name[0b10'0000'0000] = "./Assets/";  strcat(name, dirent->d_name);
		struct stat fstatus;  stat(name, &fstatus);
		char * buf = malloc(fstatus.st_size);
		FILE * file = fopen(name, "r");
		fread(buf, fstatus.st_size, 1, file);
		fclose(file);
		json_value * tree;
		if ((tree = json_parse(buf, fstatus.st_size)) != NULL) {
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
	if (assptrs.filetotal == 0)
		goto fail;

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
