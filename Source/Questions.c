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

/* This file deals with the questions portion of ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls. */

#define __STDC_WANT_IEC_60559_BFP_EXT__ 1 // Required by glibc to get SIG_ATOMIC_WIDTH for some reason.

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
#include <alloca.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include "../Libraries/mjson.h"

static char * question;
static char * answers[4];
static short correct_ans_num = -1;
static bool loadnext = true;
static signed short selected = -1;

static volatile sig_atomic_t alarm_left = 0;

static SDL_Surface * question_surface;
static SDL_Surface * answer_surfaces[4];

static SDL_Surface * response_surface;

static const char * wrong[] = {"No.", "Wrong.", "no", "Nope!", "Nope.", "Incorrect.", "Unfortunately, no."};
static const char * right[] = {"Yes.", "Yep.", "yeah", "Yep!", "Correct!"};

static bool cached;
/* `nextq_button` is used for both the skip button and the next question button—`nextq_button` is the name closest to reality that's still short and in the spirit of the language. */
/* TODO:  The quit button and timer appear in both modes.  Ideally, they would be handled by the main function instead of each mode handling them separately using the same code. */
/* TODO:  The timer buttons shouldn't use text. */
static struct buttondata * quitbutton, * backbutton, * nextq_button, * timerbuttons[3];
static SDL_Surface * timertxt_surface, * timeleft_text;

#define maxstrlenof(type) (int)ceil(sizeof (type) * CHAR_BIT * log(2) / log(10))

[[gnu::cold]] static void questions_cleanup(void)
{
	free_button(quitbutton);
	free_button(backbutton);
	free_button(nextq_button);
	for (register size_t i = 0; i < sizeof timerbuttons / sizeof timerbuttons[0]; i++)
		free_button(timerbuttons[i]);
}

static int json_arrlen(char * file, size_t filesize, char * path) {
	char * item;  int ilen;  mjson_find(file, filesize, path, (const char **)&item, &ilen);
	int off, proff;
	for (off = 0, proff = 0; (off = mjson_next(item, ilen, off, &proff, NULL, NULL, NULL, NULL)) != 0;);
	return proff;
}

[[gnu::cold]] static char * replace_substrings(char * file, size_t filesize, char * str, size_t str_len, short num)
{
	for (register unsigned long i = 0; i <= str_len; i++) {
		if (!strncmp(&str[i], "\xC2\x98", 2)) {
			char * substring = NULL;
			unsigned short y;
			bool has_ST = false;
			for (y = i + 2; y <= str_len; y++) {
				if (!strncmp(&str[y], "\xC2\x9C", 2)) {
					has_ST = true;
					break;
				}
				substring = realloc(substring, y - (i + 2));
				substring[y - (i + 2)] = str[y];
			}
			substring[y - (i + 2)] = '\0';
			if (!has_ST) {
				errno = EINVAL;
				free(substring);
				return NULL;
			}
			y += 2; // We need to keep track of where the substring ends for later; this is the first char _after_ the string terminator.
			char path[maxstrlenof (short) + (y - i) + 10];  sprintf(path, "$.data[%d].%s", num, substring);
			free(substring);
			char replacement[0xFF];
			double v;  bool ret;
			if (!mjson_get_string(file, filesize, path, replacement, 0xFF) && !((ret = mjson_get_number(file, filesize, path, &v)), strfromd(replacement, 0xFF, "%F", v), ret)) {
				errno = ENOMSG;
				return NULL;
			}
			char * betterstring = malloc(strlen(str) + strlen(replacement) + 3);
			str[i] = '\0';  strcpy(betterstring, str);  str[i] = '\xC2';  /* I tried to replace this line with a call to `strncpy()` instead, but it ended up causing memory corruption. */
			strcat(betterstring, replacement);
			strcat(betterstring, &str[y]);
			//free(replacement);
			return betterstring;
		}
	}
	return NULL;
}

void alarm_handler([[maybe_unused]] int dummy)
{
	alarm_left--;
	signal(SIGALRM, alarm_handler);
	if (alarm_left)
		alarm(1);
}

void questions_render(SDL_Surface * screen, struct assptrs assptrs, struct extra * extra, SDL_Window * window)
{
	SDL_Event e = {0};
	bool click = false;
	bool release = false;
	SDL_PumpEvents();
	while (SDL_PollEvent(&e)){
		switch (e.type){
		case SDL_QUIT:
			extra->quit = true;
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
	int w, h;  SDL_GetWindowSize(window, &w, &h);
	SDL_Rect correct_rect = (w >= 4/3 * h) ? (SDL_Rect){(w - (h * 4/3)) / 2, 0, h * 4/3, h} : (SDL_Rect){0, (h - (w * 3/4)) / 2, w, w * 3/4};
	if (clickloc.x < correct_rect.x || clickloc.y > correct_rect.x + correct_rect.w || clickloc.y < correct_rect.y || clickloc.y > correct_rect.y + correct_rect.h) {
		SDL_ShowCursor(true);
		clickloc.x = screenwidth;
		clickloc.y = screenheight;
	} else {
		SDL_ShowCursor(false);
		if (correct_rect.x > 0)
			clickloc.x -= correct_rect.x;
		else
			clickloc.y -= correct_rect.y;
		clickloc.x /= correct_rect.w * 4/3 / screenwidth;
		clickloc.y /= correct_rect.h * 4/3 / screenheight;
	}

	if (loadnext) [[clang::unlikely]] {
		__label__ fukitol;
fukitol:
		__attribute__ ((unused));  // I wasn't able to get this attribute to apply to the label with the standard syntax.
		int question_selection = rand() % json_arrlen(assptrs.curfile, assptrs.curflen, "$.questions");
		const int question_len = 0xFF;  /* MAGICALLY ARBITRARY */
		free(question);  question = malloc(question_len);
		char path[maxstrlenof (short) + 17];  sprintf(path, "$.questions[%d][0]", question_selection);
		mjson_get_string(assptrs.curfile, assptrs.curflen, path, question, question_len);
		sprintf(path, "$.questions[%d][1]", question_selection);
		for (register short i = 0; i < 4; i++) {
			free(answers[i]);
			answers[i] = malloc(question_len);
			mjson_get_string(assptrs.curfile, assptrs.curflen, path, answers[i], question_len);
		}
		correct_ans_num = rand() % (sizeof answers / sizeof answers[0] - 1);
		int x;
		for (register int i = 0; i < 4; i++) {
			char cur_ans[strlen(answers[i]) + 2];  strcpy(cur_ans, answers[i]);
			while (true) {
				free(answers[i]);
				x = rand() % json_arrlen(assptrs.curfile, assptrs.curflen, "$.data");
				if ((answers[i] = replace_substrings(assptrs.curfile, assptrs.curflen, cur_ans, strlen(cur_ans), x)) == NULL)
					continue;
				else
					break;
			}
		}
		char q[strlen(question) + 1];  strcpy(q, question);
		char cor_ans[question_len];  mjson_get_string(assptrs.curfile, assptrs.curflen, path, cor_ans, question_len);
		while (true) {
			free(question);
			free(answers[correct_ans_num]);
			x = rand() % json_arrlen(assptrs.curfile, assptrs.curflen, "$.data");
			if ((question = replace_substrings(assptrs.curfile, assptrs.curflen, q, strlen(q), x)) == NULL)
				continue;
			if ((answers[correct_ans_num] = replace_substrings(assptrs.curfile, assptrs.curflen, cor_ans, strlen(cor_ans), x)) == NULL)
				continue;
			else
				break;
		}
		for (register int i = 0; i < 4; i++)
			for (register int j = i + 1; j < 4; j++)
				if (!strcmp(answers[i], answers[j])) {
					register int toreplace = correct_ans_num == j ? i : j;
					char incor_ans[question_len];  mjson_get_string(assptrs.curfile, assptrs.curflen, path, incor_ans, question_len);
					while (true) {
						free(answers[toreplace]);
						x = rand() % json_arrlen(assptrs.curfile, assptrs.curflen, "$.data");
						if ((answers[toreplace] = replace_substrings(assptrs.curfile, assptrs.curflen, incor_ans, strlen(incor_ans), x)) == NULL)
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
		question_surface = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, question, (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, screenwidth);
		TTF_SetFontSize(assptrs.barlow_condensed, 12);
		int w;
		for (register short i = 0; i < 4; i++) {
			TTF_SizeUTF8(assptrs.barlow_condensed, answers[i], &w, NULL);
			if ((answer_surfaces[i] = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, answers[i], (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, screenwidth > w ? w : screenwidth)) == NULL)
				fprintf(stderr, "Error in %s at %d:  %s\n", __func__, __LINE__, SDL_GetError());
		}

		loadnext = false;
		alarm_left = extra->timer;  alarm(1);
	}

	if (!cached) [[clang::unlikely]] {
		quitbutton = alloc_button("Quit", (SDL_Color){0xFB, 0x49, 0x34, 0xFF}, 12, assptrs.barlow_condensed);
		quitbutton->pos[0] = 6; quitbutton->pos[1] = screenheight - 6 - quitbutton->surfaces[0]->h;
		backbutton = alloc_button("Back to main menu", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
		backbutton->pos[1] = screenheight - 6 - backbutton->surfaces[0]->h;
		backbutton->pos[0] = (screenwidth - backbutton->surfaces[0]->w) / 2;
		nextq_button = alloc_button("Skip this question", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
		nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[1]; // this is the terrible way by which it's differentiated whether the button is the skip button or the next question button; the next question button points to the second surface
		nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
		nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
		timerbuttons[0] = alloc_button("↻", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
		timerbuttons[0]->pos[0] = screenwidth - 12 - timerbuttons[0]->surfaces[0]->w * 2; timerbuttons[0]->pos[1] = screenheight - 6 - (timerbuttons[0]->surfaces[0]->h * 2);
		timerbuttons[1] = alloc_button("↑", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
		timerbuttons[1]->pos[0] = screenwidth - 6 - timerbuttons[1]->surfaces[0]->w; timerbuttons[1]->pos[1] = screenheight - 6 - (timerbuttons[1]->surfaces[0]->h * 2);
		timerbuttons[2] = alloc_button("↓", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
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
					SDL_FillRect(screen, &dest, SDL_MapRGBA(screen->format, 0x50, 0x49, 0x45, 0xFF));
				} else if (release) {
					selected = i;
					free_button(nextq_button);
					nextq_button = alloc_button("Next question", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
					nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[2]; // see above where we cached the skip button for why we're doing this
					nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
					nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
				} else {
					SDL_FillRect(screen, &dest, SDL_MapRGBA(screen->format, 0x3C, 0x38, 0x36, 0xFF));
				}
			}
		}
		dest.x += width_pad;
		SDL_BlitSurface(answer_surfaces[i], NULL, screen, &dest);
		dest.y += answer_surfaces[i]->h;
	}

	/* This doesn't work, but it appears to be harmless. */
	char txt[maxstrlenof (sig_atomic_t)];  sprintf(txt, "%d", alarm_left);
	SDL_FreeSurface(timeleft_text);
	TTF_SetFontWrappedAlign(assptrs.barlow_condensed, TTF_WRAPPED_ALIGN_CENTER);
	TTF_SetFontSize(assptrs.barlow_condensed, 16);
	timeleft_text = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, txt, (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, screenwidth);
	TTF_SetFontSize(assptrs.barlow_condensed, 12);
	dest.x = 0;  dest.y = screenheight - backbutton->surfaces[0]->h - nextq_button->surfaces[0]->h - 2 * 6;
	dest.w = timeleft_text->w;  dest.h = timeleft_text->h;
	SDL_BlitSurface(timeleft_text, NULL, screen, &dest);

	if (selected > -1) {
		if (selected != SHRT_MAX) {
			SDL_FillRect(response_surface, NULL, SDL_MapRGBA(response_surface->format, 0x28, 0x28, 0x28, 0xFF));
			SDL_Surface * rw = NULL;
			dest.y = 0;
			if (selected == correct_ans_num) {
				short i = rand() % (sizeof right / sizeof right[0] - 1);
				TTF_SetFontSize(assptrs.barlow_condensed, 16);
				rw = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, right[i], (SDL_Color){0xB8, 0xBB, 0x26, 0xFF});
				dest.x = (screenwidth - rw->w) / 2;
				dest.w = rw->w; dest.h = rw->h;
				SDL_BlitSurface(rw, NULL, response_surface, &dest);
			} else {
				if (selected != 5) {  /* i.e. the user didn't just skip the question */
					short i = rand() % (sizeof wrong / sizeof wrong[0] - 1);
					TTF_SetFontSize(assptrs.barlow_condensed, 16);
					rw = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, wrong[i], (SDL_Color){0xFB, 0x49, 0x34, 0xFF});
					dest.x = (screenwidth - rw->w) / 2;
					dest.w = rw->w; dest.h = rw->h;
					SDL_BlitSurface(rw, NULL, response_surface, &dest);
				}

				SDL_FreeSurface(rw);
				char * right_ans_txt = malloc(17 + strlen(answers[correct_ans_num]) + 1);
				sprintf(right_ans_txt, "Correct answer:  %s", answers[correct_ans_num]);
				TTF_SetFontSize(assptrs.barlow_condensed, 14);
				dest.y += (rw == NULL) ? 0 : rw->h;
				TTF_SetFontWrappedAlign(assptrs.barlow_condensed, TTF_WRAPPED_ALIGN_CENTER);
				rw = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, right_ans_txt, (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, screenwidth);
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

	BLIT_APT_BUTTONSTATE(backbutton, screen, clickloc, click, release, {
			extra->swap = true;
			free_button(nextq_button);
			nextq_button = alloc_button("Skip this question", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
			nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[1];
			nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
			nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
			selected = -1;
			loadnext = true;
		});

	if (!alarm_left && extra->timer && nextq_button->surfaces[0]->userdata == &nextq_button->surfaces[1]) {
		selected = 5;
		free_button(nextq_button);
		nextq_button = alloc_button("Next question", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
		nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[2];
		nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
		nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
	}

	if (nextq_button->surfaces[0]->userdata == &nextq_button->surfaces[1]) {
		BLIT_APT_BUTTONSTATE(nextq_button, screen, clickloc, click, release, {
				selected = 5;
				free_button(nextq_button);
				nextq_button = alloc_button("Next question", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
				nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[2];
				nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
				nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
			});
	} else {
		BLIT_APT_BUTTONSTATE(nextq_button, screen, clickloc, click, release, {
				free_button(nextq_button);
				nextq_button = alloc_button("Skip this question", (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF}, 12, assptrs.barlow_condensed);
				nextq_button->surfaces[0]->userdata = &nextq_button->surfaces[1];
				nextq_button->pos[1] = screenheight - 12 - nextq_button->surfaces[0]->h - backbutton->surfaces[0]->h;
				nextq_button->pos[0] = (screenwidth - nextq_button->surfaces[0]->w) / 2;
				selected = -1;
				loadnext = true;
			});
	}

	BLIT_APT_BUTTONSTATE(quitbutton, screen, clickloc, click, release, {SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});}); // could just directly set the quit state, but this is sillier and more fun

	BLIT_APT_BUTTONSTATE(timerbuttons[0], screen, clickloc, click, release, {extra->timer = 0;});
	BLIT_APT_BUTTONSTATE(timerbuttons[2], screen, clickloc, click, release, {if (extra->timer != 0) extra->timer--;});
	BLIT_APT_BUTTONSTATE(timerbuttons[1], screen, clickloc, click, release, {extra->timer++;});

	char timertxt[15 + 6 + 1]; // space for words + 16-bit largest integer length as string + NUL
	if (extra->timer != 0)
		sprintf(timertxt, "Timer: %d seconds", extra->timer);
	else
		sprintf(timertxt, "No timer");
	SDL_FreeSurface(timertxt_surface);
	TTF_SetFontSize(assptrs.barlow_condensed, 12);
	timertxt_surface = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, timertxt, (SDL_Color){0xEB, 0xDB, 0xB2, 0xFF});
	dest.y = timerbuttons[2]->pos[1]; dest.x = timerbuttons[2]->pos[0] - timertxt_surface->w - 6;
	dest.w = timertxt_surface->w; dest.h = timertxt_surface->h;
	SDL_BlitSurface(timertxt_surface, NULL, screen, &dest);

	SDL_Rect mptr = {.x = clickloc.x, .y = clickloc.y, .w = 8, .h = 4};
	SDL_FillRect(screen, &mptr, SDL_MapRGBA(screen->format, 0xFB, 0x49, 0x34, 0xFF));
	mptr.w = 4;  mptr.h = 8;
	SDL_FillRect(screen, &mptr, SDL_MapRGBA(screen->format, 0xFB, 0x49, 0x34, 0xFF));

	if (extra->quit)
		questions_cleanup();
}
