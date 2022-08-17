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

/* This file deals with the questions portion of ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls. */

#include "SDL.h"
#include "Questions.h"
#include "Init.h"
#include "SDL2/SDL_ttf.h"
#include "Gui.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h> //TODO:  Once we can move to Clang 15, get this out of here.

static char * question;
static char * answers[4];
static short correct_ans_num = -1;
static bool loadnext = true;
static signed short selected = -1;

static SDL_Surface * question_surface;
static SDL_Surface * answer_surfaces[4];

static SDL_Surface * response_surface;

static const char * wrong[] = {"No.", "Wrong.", "no", "Nope!", "Nope.", "Incorrect.", "Unfortunately, no."};
static const char * right[] = {"Yes.", "Yep.", "yeah", "Yep!", "Correct!"};

static bool cached;
/* `nextq_button` is used for both the skip button and the next question button—`nextq_button` is the name closest to reality that's still short and in the spirit of the language. */
/* TODO:  The quit button and timer appear in both modes.  Ideally, they would be handled by the main function instead of each mode handling them separately using the same code. */
static struct buttondata * quitbutton, * backbutton, * nextq_button, * timerbuttons[3];
static SDL_Surface * timertxt_surface;

[[gnu::cold]] static void questions_cleanup(void)
{
	free_button(quitbutton);
	free_button(backbutton);
	free_button(nextq_button);
	for (register size_t i = 0; i < sizeof timerbuttons / sizeof timerbuttons[0]; i++)
		free_button(timerbuttons[i]);
}

#if __has_c_attribute(unsequenced)
[[unsequenced]]
#endif
/* This is a bit of an unintuitive interface, but i couldn't figure out how to get the type of the unnamed struct directly. */
static signed int jsongetkey(const char * req_key, json_value * obj)
{
	for (register unsigned int i = 0; i < obj->u.object.length; i++)
		if (!strcmp(req_key, obj->u.object.values[i].name))
			return i;
	return -1;
}

[[gnu::cold]] static char * replace_substrings(json_value * file, char * str, short num)
{
	for (register unsigned long i = 0; i < strlen(str); i++) {
		if (str[i] == '\xc2' && str[i + 1] == '\x98') { // Direct comparison of UTF-8 bytes due to restrictions on control-character unicode escape sequences—this is also the case a few lines down.
			char * substring = NULL;
			unsigned short y;
			bool has_ST = false;
			for (y = i + 2; y < strlen(str); y++) {
				if (str[y] == '\xc2' && str[y + 1] == '\x9c') {
					has_ST = true;
					break;
				}
				substring = realloc(substring, y - (i + 2));
				substring[y - (i + 2)] = str[y];
			}
			substring[y - (i + 2)] = '\0';
			if (!has_ST) {
				errno = EINVAL;
				return NULL;
			}
			y += 2; // We need to keep track of where the substring ends for later; this is the first char _after_ the string terminator.
			signed int dataloc;
			if ((dataloc = jsongetkey("data", file)) == -1) {
				fprintf(stderr, "\033[31mA critical error has occurred in the function %s due to an invalid JSON file (missing the data section) being supplied to the program.  The program will crash now.  Apologies for the inconvenience.\033[m\n", __func__);
				exit(EXIT_FAILURE);
			}
			int substr_loc = -1;
			if ((substr_loc = jsongetkey(substring, file->u.object.values[dataloc].value->u.array.values[num])) == -1) {
				errno = ENOMSG;
				return NULL;
			}
			free(substring);
			char * replacement;
			switch (file->u.object.values[dataloc].value->u.array.values[num]->u.object.values[substr_loc].value->type) {
			case json_integer:
				replacement = __builtin_alloca(20 + 1); // maximum length of 64-bit integer + nul char
				sprintf(replacement, "%" PRIdFAST64 "", file->u.object.values[dataloc].value->u.array.values[num]->u.object.values[substr_loc].value->u.integer);
				break;
			case json_string:
				replacement = __builtin_alloca(file->u.object.values[dataloc].value->u.array.values[num]->u.object.values[substr_loc].value->u.string.length + 1);
				strcpy(replacement, file->u.object.values[dataloc].value->u.array.values[num]->u.object.values[substr_loc].value->u.string.ptr);
				break;
			default:
			case json_null:
				errno = ENOMSG;
				return NULL;
			}
			char * betterstring = malloc(strlen(str) + strlen(replacement) + 2);
			str[i] = '\0'; // One potential concern that i have with this method of doing things is that i'm worried that there's going to be some sort of optimization whereupon the compiler will think that since i've ended the string at this point now it's now okay to use the bits of the string beyond this point for other stuff.  I mean, i don't think that this is happening at the moment, but with programs getting shittier despite increasing demand for performance, i wouldn't be surprised if the people designing the optimizers start to get desperate.
			strcpy(betterstring, str);
			str[i] = '\xc2';
			strcat(betterstring, replacement);
			strcat(betterstring, &str[y]);
			return betterstring;
		}
	}
	return NULL;
}

void questions_render(SDL_Surface * screen, struct assptrs assptrs)
{
	SDL_Event e = {0};
	bool click = false;
	bool release = false;
	SDL_PumpEvents();
	while (SDL_PollEvent(&e)){
		switch (e.type){
		case SDL_QUIT:
			((struct extra *)screen->userdata)->quit = true;
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.button == SDL_BUTTON_LEFT)
				release = true;
		}
	}
	/* TODO:  scaling.  This will also require work in `Source/Main.c` and `Source/Menu.c`. */
	SDL_Point clickloc;
	SDL_PumpEvents();
	click = (SDL_GetMouseState(&clickloc.x, &clickloc.y) & SDL_BUTTON_LMASK) ? true : false;

	if (loadnext) [[clang::unlikely]] {
		__label__ fukitol;
fukitol:
		__attribute__((unused));  // I wasn't able to get this attribute to apply to the label with the standard syntax.
		int questions_loc = jsongetkey("questions", assptrs.curfile);
		if (questions_loc == -1) {
			fprintf(stderr, "\033[31mA catastrophic error has occurred in the function %s due to an invalid JSON file (missing the questions) being supplied to the program.  The program will crash now.  Apologies for the inconvenience.\033[m\n", __func__);
			exit(EXIT_FAILURE);
		}
		int selection_loc = rand() % (assptrs.curfile->u.object.values[questions_loc].value->u.array.length - 1);
		free(question);
		question = malloc(assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[0]->u.string.length + 1);
		strcpy(question, assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[0]->u.string.ptr);
		for (register short i = 0; i < 4; i++) {
			free(answers[i]);
			answers[i] = malloc(assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[1]->u.string.length + 1);
			strcpy(answers[i], assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[1]->u.string.ptr);
		}
		correct_ans_num = rand() % (sizeof answers / sizeof answers[0] - 1);
		int x;
		int data_loc = jsongetkey("data", assptrs.curfile);
		if (data_loc == -1) {
			fprintf(stderr, "\033[31mA critical error has occurred in the function %s due to an invalid JSON file (missing the data section) being supplied to the program.  The program will crash now.  Apologies for the inconvenience.\033[m\n", __func__);
			exit(EXIT_FAILURE);
		}
		for (register int i = 0; i < 4; i++) {
			char cur_ans[strlen(answers[i]) + 2];  strcpy(cur_ans, answers[i]);
			while (true) {
				free(answers[i]);
				x = rand() % (assptrs.curfile->u.object.values[data_loc].value->u.array.length - 1);
				if ((answers[i] = replace_substrings(assptrs.curfile, cur_ans, x)) == NULL)
					continue;
				else
					break;
			}
		}
		char * q = __builtin_alloca(strlen(question) + 1);  strcpy(q, question);
		char * cor_ans = __builtin_alloca(assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[1]->u.string.length + 1);  strcpy(cor_ans, assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[1]->u.string.ptr);
		while (true) {
			free(question);
			free(answers[correct_ans_num]);
			x = rand() % (assptrs.curfile->u.object.values[data_loc].value->u.array.length - 1);
			if ((question = replace_substrings(assptrs.curfile, q, x)) == NULL)
				continue;
			if ((answers[correct_ans_num] = replace_substrings(assptrs.curfile, cor_ans, x)) == NULL)
				continue;
			else
				break;
		}
		for (register int i = 0; i < 4; i++)
			for (register int j = i + 1; j < 4; j++)
				if (!strcmp(answers[i], answers[j])) {
					register int toreplace = correct_ans_num == j ? i : j;
					char incor_ans[assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[1]->u.string.length + 1];  strcpy(cor_ans, assptrs.curfile->u.object.values[questions_loc].value->u.array.values[selection_loc]->u.array.values[1]->u.string.ptr);
					while (true) {
						free(answers[toreplace]);
						x = rand() % (assptrs.curfile->u.object.values[data_loc].value->u.array.length - 1);
						if ((answers[toreplace] = replace_substrings(assptrs.curfile, incor_ans, x)) == NULL)
							continue;
						else
							break;
					}
				}

		SDL_FreeSurface(question_surface);
		for (register short i = 0; i < 4; i++)
			SDL_FreeSurface(answer_surfaces[i]);

		TTF_SetFontWrappedAlign(assptrs.barlow_condensed, TTF_WRAPPED_ALIGN_CENTER);
		TTF_SetFontSize(assptrs.barlow_condensed, 16);
		question_surface = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, question, (SDL_Color){0xff, 0xff, 0xff, 0xff}, screenwidth);
		TTF_SetFontSize(assptrs.barlow_condensed, 12);
		int w;
		for (register short i = 0; i < 4; i++) {
			TTF_SizeUTF8(assptrs.barlow_condensed, answers[i], &w, NULL);
			if ((answer_surfaces[i] = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, answers[i], (SDL_Color){0xff, 0xff, 0xff, 0xff}, screenwidth > w ? w : screenwidth)) == NULL)
				fprintf(stderr, "Error in %s at %d:  %s\n", __func__, __LINE__, SDL_GetError());
		}

		loadnext = false;
	}

	if (!cached) [[clang::unlikely]] {
		quitbutton = alloc_button("Quit", (SDL_Color){0xff, 0x44, 0x44, 0xff}, 12, assptrs.barlow_condensed);
		quitbutton->pos[0] = 6; quitbutton->pos[1] = screenheight - 6 - quitbutton->surfaces[0]->h;
		backbutton = alloc_button("Back to main menu", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		backbutton->pos[1] = screenheight - 6 - backbutton->surfaces[0]->h;
		backbutton->pos[0] = (screenwidth - backbutton->surfaces[0]->w) / 2;
		nextq_button = alloc_button("Skip this question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[1]; // this is the terrible way by which it's differentiated whether the button is the skip button or the next question button; the next question button points to the second surface
		nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
		nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
		timerbuttons[0] = alloc_button("↻", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timerbuttons[0]->pos[0] = screenwidth - 12 - timerbuttons[0]->surfaces[0]->w * 2; timerbuttons[0]->pos[1] = screenheight - 6 - (timerbuttons[0]->surfaces[0]->h * 2);
		timerbuttons[1] = alloc_button("↑", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timerbuttons[1]->pos[0] = screenwidth - 6 - timerbuttons[1]->surfaces[0]->w; timerbuttons[1]->pos[1] = screenheight - 6 - (timerbuttons[1]->surfaces[0]->h * 2);
		timerbuttons[2] = alloc_button("↓", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timerbuttons[2]->pos[0] = screenwidth - 6 - timerbuttons[2]->surfaces[0]->w; timerbuttons[2]->pos[1] = screenheight - 6 - timerbuttons[2]->surfaces[0]->h;
		response_surface = SDL_CreateRGBSurfaceWithFormat(0, screenwidth, screenheight / 2, 32, SDL_PIXELFORMAT_RGBA32);
		cached = true;
	}

	SDL_Rect dest = {0};

	dest.x = (screenwidth - question_surface->w) / 2; dest.y = 0;
	dest.w = question_surface->w; dest.h = question_surface->h;
	SDL_BlitSurface(question_surface, NULL, screen, &dest);

	/* This can't use the horrifying macro in `Gui.h` because the answers aren't stored as `struct buttondata`.  Quite honestly, that could be considered a mercy of fate.  Most everything else can use the macro. */
	dest.y += question_surface->h * 3/2;
	for (register short i = 0; i < 4; i++) {
		const short width_pad = 6;
		dest.x = (screenwidth - answer_surfaces[i]->w) / 2 - width_pad;
		dest.w = answer_surfaces[i]->w + 2 * width_pad; dest.h = answer_surfaces[i]->h;
		if (selected < 0) {
			if (SDL_PointInRect(&clickloc, &dest)) {
				if (click) {
					SDL_FillRect(screen, &dest, SDL_MapRGBA(screen->format, 0x88, 0x88, 0x88, 0xff));
				} else if (release) {
					selected = i;
					free_button(nextq_button);
					nextq_button = alloc_button("Next question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
					nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[2]; // see above where we cached the skip button for why we're doing this
					nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
					nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
				} else {
					SDL_FillRect(screen, &dest, SDL_MapRGBA(screen->format, 0x44, 0x44, 0x44, 0xff));
				}
			}
		}
		dest.x += width_pad;
		SDL_BlitSurface(answer_surfaces[i], NULL, screen, &dest);
		dest.y += answer_surfaces[i]->h;
	}

	if (selected > -1) {
		if (selected != SHRT_MAX) {
			SDL_FillRect(response_surface, NULL, SDL_MapRGBA(response_surface->format, 0, 0, 0, 0xff));
			SDL_Surface * rw = NULL;
			dest.y = 0;
			if (selected == correct_ans_num) {
				short i = rand() % (sizeof right / sizeof right[0] - 1);
				TTF_SetFontSize(assptrs.barlow_condensed, 16);
				rw = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, right[i], (SDL_Color){0x44, 0xff, 0x44, 0xff});
				dest.x = (screenwidth - rw->w) / 2;
				dest.w = rw->w; dest.h = rw->h;
				SDL_BlitSurface(rw, NULL, response_surface, &dest);
			} else {
				short i = rand() % (sizeof wrong / sizeof wrong[0] - 1);
				TTF_SetFontSize(assptrs.barlow_condensed, 16);
				rw = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, wrong[i], (SDL_Color){0xff, 0x44, 0x44, 0xff});
				dest.x = (screenwidth - rw->w) / 2;
				dest.w = rw->w; dest.h = rw->h;
				SDL_BlitSurface(rw, NULL, response_surface, &dest);

				SDL_FreeSurface(rw);
				char * right_ans_txt = malloc(17 + strlen(answers[correct_ans_num]) + 1);
				sprintf(right_ans_txt, "Correct answer:  %s", answers[correct_ans_num]);
				TTF_SetFontSize(assptrs.barlow_condensed, 14);
				dest.y += rw->h;
				TTF_SetFontWrappedAlign(assptrs.barlow_condensed, TTF_WRAPPED_ALIGN_CENTER);
				rw = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, right_ans_txt, (SDL_Color){0xff, 0xff, 0xff, 0xff}, screenwidth);
				dest.x = (screenwidth - rw->w) / 2;
				dest.w = rw->w; dest.h = rw->h;
				SDL_BlitSurface(rw, NULL, response_surface, &dest);
			}
			SDL_FreeSurface(rw);
			selected = SHRT_MAX;
		}
		dest.x = (screenwidth - response_surface->w) / 2; dest.y = screenheight / 2;
		dest.w = response_surface->w; dest.h = response_surface->h;
		SDL_BlitSurface(response_surface, NULL, screen, &dest);
	}

	BLIT_APT_BUTTONSTATE(backbutton, screen, clickloc, click, release, {((struct extra *)screen->userdata)->swap = true;});

	if (nextq_button->surfaces[0]->userdata == &nextq_button->surfaces[1]) {
		BLIT_APT_BUTTONSTATE(nextq_button, screen, clickloc, click, release, {
				selected = 5;
				free_button(nextq_button);
				nextq_button = alloc_button("Next question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
				nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[2];
				nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
				nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
			});
	} else {
		BLIT_APT_BUTTONSTATE(nextq_button, screen, clickloc, click, release, {
				free_button(nextq_button);
				nextq_button = alloc_button("Skip this question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
				nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[1];
				nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
				nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
				selected = -1;
				loadnext = true;
			});
	}

	BLIT_APT_BUTTONSTATE(quitbutton, screen, clickloc, click, release, {SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});}); // could just directly set the quit state, but this is sillier and more fun

	BLIT_APT_BUTTONSTATE(timerbuttons[0], screen, clickloc, click, release, {((struct extra *)screen->userdata)->timer = 0;});
	BLIT_APT_BUTTONSTATE(timerbuttons[2], screen, clickloc, click, release, {if (((struct extra *)screen->userdata)->timer != 0) ((struct extra *)screen->userdata)->timer--;});
	BLIT_APT_BUTTONSTATE(timerbuttons[1], screen, clickloc, click, release, {((struct extra *)screen->userdata)->timer++;});

	char timertxt[7 + 6 + 1]; // space for words + 16-bit largest integer length as string + NUL
	if (((struct extra *)screen->userdata)->timer != 0)
		sprintf(timertxt, "Timer: %d", ((struct extra *)(screen->userdata))->timer);
	else
		sprintf(timertxt, "No timer");
	SDL_FreeSurface(timertxt_surface);
	TTF_SetFontSize(assptrs.barlow_condensed, 12);
	timertxt_surface = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, timertxt, (SDL_Color){0xff, 0xff, 0xff, 0xff});
	dest.y = timerbuttons[2]->pos[1]; dest.x = timerbuttons[2]->pos[0] - timertxt_surface->w - 6;
	dest.w = timertxt_surface->w; dest.h = timertxt_surface->h;
	SDL_BlitSurface(timertxt_surface, NULL, screen, &dest);

	if (((struct extra *)screen->userdata)->quit) questions_cleanup();
}
