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
#include <stdbool.h> //TODO:  Once we can move to Clang 15, get this out of here.

/* These are hardcoded for now.  Once we get the json parsing up and running, these will be done correctly. */
static char * question = "What is the meaning of \"ļļč\"?";
static char * answers[4] = {
	"\"(sigh) Fuck.\"",
	"\"As luck would have it…\"",
	"\"Holy fucking shit!\"",
	"\"This is your last chance: …\""
};
static short correct_ans_num = 2;
static bool loadnext = true;
static signed short selected = -1;

static SDL_Surface * question_surface;
static SDL_Surface * answer_surfaces[4];

static SDL_Surface * finale;

static const char * wrong[] = {"No.", "Wrong.", "no", "Nope!", "Nope.", "Incorrect.", "Unfortunately, no."};
static const char * right[] = {"Yes.", "Yep.", "yeah", "Yep!", "Correct!"};

static bool cached;
struct buttondata {int x; int y; SDL_Surface * surfaces[3];};
static struct buttondata * quitbutton, * backbutton, * skipbutton, * timerbuttons[3];
static SDL_Surface * timertxt_surface;

[[gnu::cold]] static void free_button (struct buttondata * button)
{
	for (register short i = 0; i < 3; i++)
		SDL_FreeSurface(button->surfaces[i]);
	free(button);
}

[[gnu::malloc]] [[gnu::cold]] static struct buttondata * alloc_button(const char * text, SDL_Color color, short ptsize, TTF_Font * font)
{
	int h, w;
	TTF_SetFontSize(font, ptsize);
	TTF_SizeUTF8(font, text, &w, &h);
	struct buttondata * button = malloc(sizeof (struct buttondata));
	for (register short i = 0; i < 3; i++)
		button->surfaces[i] = draw_button_with_text((enum buttonstate) i, text, (SDL_Rect){.h = h, .w = w + 6, .x = 0, .y = 0}, font, 12, color);
	return button;
}

static void questions_cleanup(void){
	free_button(quitbutton);
	free_button(backbutton);
	free_button(skipbutton);
	for (register short i = 0; i < 3; i++)
		free_button(timerbuttons[i]);
}

void questions_render(SDL_Surface * screen, struct assptrs assptrs)
{
	SDL_Event e;
	int mousepos[2] = {0};
	bool click = false;
	bool release = false;
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
	SDL_PumpEvents();
	click = (SDL_GetMouseState(&mousepos[0], &mousepos[1]) & SDL_BUTTON_LMASK) ? true : false;

	if (loadnext) [[clang::unlikely]] {
		/* once JSON parsing is implemented, loading the next question should go about here */
		SDL_FreeSurface(question_surface);
		for (register short i = 0; i < 4; i++)
			SDL_FreeSurface(answer_surfaces[i]);

		TTF_SetFontSize(assptrs.barlow_condensed, 16);
		question_surface = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, question, (SDL_Color){0xff, 0xff, 0xff, 0xff}, screenwidth);
		TTF_SetFontSize(assptrs.barlow_condensed, 12);
		for (register short i = 0; i < 4; i++)
			answer_surfaces[i] = TTF_RenderUTF8_Blended_Wrapped(assptrs.barlow_condensed, answers[i], (SDL_Color){0xff, 0xff, 0xff, 0xff}, screenwidth);

		loadnext = false;
	}

	if (!__builtin_expect_with_probability(cached, true, 1.0)) [[clang::unlikely]] {
		quitbutton = alloc_button("Quit", (SDL_Color){0xff, 0x44, 0x44, 0xff}, 12, assptrs.barlow_condensed);
		quitbutton->x = 6; quitbutton->y = screenheight - 6 - quitbutton->surfaces[0]->h;
		backbutton = alloc_button("Back to main menu", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		backbutton->y = screenheight - 6 - backbutton->surfaces[0]->h;
		backbutton->x = (screenwidth - backbutton->surfaces[0]->w) / 2;
		skipbutton = alloc_button("Skip this question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		skipbutton->y = screenheight - 12 - skipbutton->surfaces[0]->h - backbutton->surfaces[0]->h;
		skipbutton->x = (screenwidth - skipbutton->surfaces[0]->w) / 2;
		timerbuttons[0] = alloc_button("↻", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timerbuttons[0]->x = screenwidth - 12 - timerbuttons[0]->surfaces[0]->w * 2; timerbuttons[0]->y = screenheight - 6 - (timerbuttons[0]->surfaces[0]->h * 2);
		timerbuttons[1] = alloc_button("↑", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timerbuttons[1]->x = screenwidth - 6 - timerbuttons[0]->surfaces[0]->w; timerbuttons[1]->y = screenheight - 6 - (timerbuttons[1]->surfaces[0]->h * 2);
		timerbuttons[2] = alloc_button("↓", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
		timerbuttons[2]->x = screenwidth - 6 - timerbuttons[2]->surfaces[0]->w; timerbuttons[2]->y = screenheight - 6 - timerbuttons[2]->surfaces[0]->h;
		finale = SDL_CreateRGBSurfaceWithFormat(0, screenwidth, screenheight / 2, 32, SDL_PIXELFORMAT_RGBA32);
		cached = true;
	}

	SDL_Rect dest = {0};
	SDL_Point clickloc = {.x = mousepos[0], .y = mousepos[1]};

	dest.x = (screenwidth - question_surface->w) / 2; dest.y = 0;
	dest.w = question_surface->w; dest.h = question_surface->h;
	SDL_BlitSurface(question_surface, NULL, screen, &dest);
	dest.y += question_surface->h * 3/2;
	for (register short i = 0; i < 4; i++) {
		dest.x = (screenwidth - answer_surfaces[i]->w) / 2;
		dest.w = answer_surfaces[i]->w; dest.h = answer_surfaces[i]->h;
		if (selected < 0) {
			if (SDL_PointInRect(&clickloc, &dest)) {
				if (click) {
					SDL_FillRect(screen, &dest, SDL_MapRGBA(screen->format, 0x88, 0x88, 0x88, 0xff));
				} else if (release) {
					selected = i;
					free_button(skipbutton);
					skipbutton = alloc_button("Next question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
					skipbutton->y = screenheight - 12 - skipbutton->surfaces[0]->h - backbutton->surfaces[0]->h;
					skipbutton->x = (screenwidth - skipbutton->surfaces[0]->w) / 2;
				} else {
					SDL_FillRect(screen, &dest, SDL_MapRGBA(screen->format, 0x44, 0x44, 0x44, 0xff));
				}
			}
		}
		SDL_BlitSurface(answer_surfaces[i], NULL, screen, &dest);
		dest.y += answer_surfaces[i]->h;
	}

	if (selected > -1) {
		if (selected != SHRT_MAX) {
			SDL_FillRect(finale, NULL, SDL_MapRGBA(finale->format, 0, 0, 0, 0xff));
			SDL_Surface * rw = NULL;
			dest.y = 0;
			if (selected == correct_ans_num) {
				short i = rand() % (sizeof right / sizeof right[i]);
				TTF_SetFontSize(assptrs.barlow_condensed, 16);
				rw = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, right[i], (SDL_Color){0x44, 0xff, 0x44, 0xff});
				dest.x = (screenwidth - rw->w) / 2;
				dest.w = rw->w; dest.h = rw->h;
				SDL_BlitSurface(rw, NULL, finale, &dest);
				SDL_FreeSurface(rw);
			} else {
				short i = rand() % (sizeof wrong / sizeof wrong[i]);
				TTF_SetFontSize(assptrs.barlow_condensed, 16);
				rw = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, wrong[i], (SDL_Color){0xff, 0x44, 0x44, 0xff});
				dest.x = (screenwidth - rw->w) / 2;
				dest.w = rw->w; dest.h = rw->h;
				SDL_BlitSurface(rw, NULL, finale, &dest);
				SDL_FreeSurface(rw);
				char right_ans_txt[32] = "";
				snprintf(right_ans_txt, sizeof right_ans_txt, "Correct answer:  %d", correct_ans_num);
				TTF_SetFontSize(assptrs.barlow_condensed, 14);
				dest.y += rw->h;
				rw = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, right_ans_txt, (SDL_Color){0xff, 0xff, 0xff, 0xff});
				dest.x = (screenwidth - rw->w) / 2;
				dest.w = rw->w; dest.h = rw->h;
				SDL_BlitSurface(rw, NULL, finale, &dest);
			}
			selected = SHRT_MAX;
		}
		dest.x = (screenwidth - finale->w) / 2; dest.y = screenheight / 2;
		dest.w = finale->w; dest.h = finale->h;
		SDL_BlitSurface(finale, NULL, screen, &dest);
	}

	dest.x = backbutton->x; dest.y = backbutton->y;
	dest.w = backbutton->surfaces[0]->w; dest.h = backbutton->surfaces[0]->h;
	if (SDL_PointInRect(&clickloc, &dest)) {
		if (click) SDL_BlitSurface(backbutton->surfaces[2], NULL, screen, &dest);
		else if (release) ((struct extra *)(screen->userdata))->swap = true;
		else SDL_BlitSurface(backbutton->surfaces[1], NULL, screen, &dest);
	} else {
		SDL_BlitSurface(backbutton->surfaces[0], NULL, screen, &dest);
	}

	dest.x = skipbutton->x; dest.y = skipbutton->y;
	dest.w = skipbutton->surfaces[0]->w; dest.h = skipbutton->surfaces[0]->h;
	if (SDL_PointInRect(&clickloc, &dest)) {
		if (click) {
			SDL_BlitSurface(skipbutton->surfaces[2], NULL, screen, &dest);
		} else {
			int w_next, w_skip;
			TTF_SizeUTF8(assptrs.barlow_condensed, "Next question", &w_next, NULL);
			TTF_SizeUTF8(assptrs.barlow_condensed, "Skip this question", &w_skip, NULL);
			if (release) {
				if (skipbutton->surfaces[0]->w == w_skip + 6) {
					selected = 5;
					free_button(skipbutton);
					skipbutton = alloc_button("Next question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
					skipbutton->y = screenheight - 12 - skipbutton->surfaces[0]->h - backbutton->surfaces[0]->h;
					skipbutton->x = (screenwidth - skipbutton->surfaces[0]->w) / 2;
				} else if (skipbutton->surfaces[0]->w == w_next + 6){
					free_button(skipbutton);
					skipbutton = alloc_button("Skip this question", (SDL_Color){0xff, 0xff, 0xff, 0xff}, 12, assptrs.barlow_condensed);
					skipbutton->y = screenheight - 12 - skipbutton->surfaces[0]->h - backbutton->surfaces[0]->h;
					skipbutton->x = (screenwidth - skipbutton->surfaces[0]->w) / 2;
					selected = -1;
				}
			}
			loadnext = release ? true : false;
			SDL_BlitSurface(skipbutton->surfaces[1], NULL, screen, &dest);
		}
	} else {
		SDL_BlitSurface(skipbutton->surfaces[0], NULL, screen, &dest);
	}

	dest.x = quitbutton->x; dest.y = quitbutton->y;
	dest.w = quitbutton->surfaces[0]->w; dest.h = quitbutton->surfaces[0]->h;
	if (SDL_PointInRect(&clickloc, &dest)) {
		if (click) SDL_BlitSurface(quitbutton->surfaces[2], NULL, screen, &dest);
		else if (release) SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});  // could just directly set the quit state, but this is sillier and more fun
		else SDL_BlitSurface(quitbutton->surfaces[1], NULL, screen, &dest);
	} else {
		SDL_BlitSurface(quitbutton->surfaces[0], NULL, screen, &dest);
	}

	for (register short i = 0; i < 3; i++) {
		dest.x = timerbuttons[i]->x; dest.y = timerbuttons[i]->y;
		dest.w = timerbuttons[i]->surfaces[0]->w; dest.h = timerbuttons[i]->surfaces[0]->h;
		if (SDL_PointInRect(&clickloc, &dest)) {
			if (click) {
				SDL_BlitSurface(timerbuttons[i]->surfaces[2], NULL, screen, &dest);
			} else {
				if (release) switch (i){
				case 0:
					((struct extra *)(screen->userdata))->timer = 0;
					break;
				case 2:
					if (((struct extra *)(screen->userdata))->timer != 0)
						((struct extra *)(screen->userdata))->timer--;
					break;
				case 1:
					((struct extra *)(screen->userdata))->timer++;
					break;
				}
				SDL_BlitSurface(timerbuttons[i]->surfaces[1], NULL, screen, &dest);
			}
		} else {
			SDL_BlitSurface(timerbuttons[i]->surfaces[0], NULL, screen, &dest);
		}
	}
	char timertxt[7 + 6 + 1]; // space for words + 16-bit largest integer length as string + NUL
	if (((struct extra *)(screen->userdata))->timer != 0)
		sprintf(timertxt, "Timer: %d", ((struct extra *)(screen->userdata))->timer);
	else
		sprintf(timertxt, "No timer");
	SDL_FreeSurface(timertxt_surface);
	TTF_SetFontSize(assptrs.barlow_condensed, 12);
	timertxt_surface = TTF_RenderUTF8_Blended(assptrs.barlow_condensed, timertxt, (SDL_Color){0xff, 0xff, 0xff, 0xff});
	dest.y = timerbuttons[2]->y; dest.x = timerbuttons[2]->x - timertxt_surface->w - 6;
	dest.w = timertxt_surface->w; dest.h = timertxt_surface->h;
	SDL_BlitSurface(timertxt_surface, NULL, screen, &dest);

	if (((struct extra *)screen->userdata)->quit) questions_cleanup();
}
